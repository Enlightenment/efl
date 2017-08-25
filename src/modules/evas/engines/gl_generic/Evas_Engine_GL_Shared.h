#ifndef EVAS_ENGINE_GL_SHARED_H
#define EVAS_ENGINE_GL_SHARED_H

#include "../software_generic/Evas_Engine_Software_Shared.h"

typedef enum _Evas_Engine_Info_Gl_Swap_Mode
{
   EVAS_ENGINE_GL_SWAP_MODE_AUTO = 0,
   EVAS_ENGINE_GL_SWAP_MODE_FULL = 1,
   EVAS_ENGINE_GL_SWAP_MODE_COPY = 2,
   EVAS_ENGINE_GL_SWAP_MODE_DOUBLE = 3,
   EVAS_ENGINE_GL_SWAP_MODE_TRIPLE = 4,
   EVAS_ENGINE_GL_SWAP_MODE_QUADRUPLE = 5
} Evas_Engine_Info_Gl_Swap_Mode;

static inline Render_Output_Swap_Mode
evas_render_engine_gl_swap_mode_get(Evas_Engine_Info_Gl_Swap_Mode info_swap_mode)
{
   Render_Output_Swap_Mode swap_mode = MODE_FULL;
   const char *s;

   s = getenv("EVAS_GL_SWAP_MODE");
   if (s)
     {
        if ((!strcasecmp(s, "full")) || (!strcasecmp(s, "f")))
          swap_mode = MODE_FULL;
        else if ((!strcasecmp(s, "copy")) || (!strcasecmp(s, "c")))
          swap_mode = MODE_COPY;
        else if ((!strcasecmp(s, "double")) ||
                 (!strcasecmp(s, "d")) || (!strcasecmp(s, "2")))
          swap_mode = MODE_DOUBLE;
        else if ((!strcasecmp(s, "triple")) ||
                 (!strcasecmp(s, "t")) || (!strcasecmp(s, "3")))
          swap_mode = MODE_TRIPLE;
        else if ((!strcasecmp(s, "quadruple")) ||
                 (!strcasecmp(s, "q")) || (!strcasecmp(s, "4")))
          swap_mode = MODE_QUADRUPLE;
     }
   else
     {
// in most gl implementations - egl and glx here that we care about the TEND
// to either swap or copy backbuffer and front buffer, but strictly that is
// not true. technically backbuffer content is totally undefined after a swap
// and thus you MUST re-render all of it, thus MODE_FULL
        swap_mode = MODE_FULL;
// BUT... reality is that lmost every implementation copies or swaps so
// triple buffer mode can be used as it is a superset of double buffer and
// copy (though using those explicitly is more efficient). so let's play with
// triple buffer mdoe as a default and see.
//        re->mode = MODE_TRIPLE;
// XXX: note - the above seems to break on some older intel chipsets and
// drivers. it seems we CANT depend on backbuffer staying around. bugger!
        switch (info_swap_mode)
          {
           case EVAS_ENGINE_GL_SWAP_MODE_FULL:
             swap_mode = MODE_FULL;
             break;
           case EVAS_ENGINE_GL_SWAP_MODE_COPY:
             swap_mode = MODE_COPY;
             break;
           case EVAS_ENGINE_GL_SWAP_MODE_DOUBLE:
             swap_mode = MODE_DOUBLE;
             break;
           case EVAS_ENGINE_GL_SWAP_MODE_TRIPLE:
             swap_mode = MODE_TRIPLE;
             break;
           case EVAS_ENGINE_GL_SWAP_MODE_QUADRUPLE:
             swap_mode = MODE_QUADRUPLE;
             break;
           default:
             swap_mode = MODE_AUTO;
             break;
          }
     }

   return swap_mode;
}

#endif

