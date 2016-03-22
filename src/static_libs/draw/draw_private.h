#ifndef EFL_DRAW_PRIVATE_H
#define EFL_DRAW_PRIVATE_H

#include "draw.h"

#define DRAW_BYTE_MUL(c, a) \
 ( (((((c) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + \
   (((((c) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff) )

#define LOOP_ALIGNED_U1_A4(DEST, LENGTH, UOP, A4OP) \
   { \
      while((uintptr_t)DEST & 0xF && LENGTH) UOP \
   \
      while(LENGTH) { \
         switch(LENGTH) { \
            case 3: \
            case 2: \
            case 1: \
               UOP \
               break; \
            default: \
               A4OP \
               break; \
         } \
      } \
   }

// optimization
#define DIV_USING_BITSHIFT 1
// behaviour setting
#define RGBA2LUM_WEIGHTED 1

/* 255 - alpha */
static inline int
alpha_inverse(uint32_t color)
{
   color = ~color;
   return A_VAL(&color);
}

extern RGBA_Comp_Func_Solid func_for_mode_solid[EFL_GFX_RENDER_OP_LAST];
extern RGBA_Comp_Func func_for_mode[EFL_GFX_RENDER_OP_LAST];
extern int _draw_log_dom;

void efl_draw_sse2_init(void);
void efl_draw_neon_init(void);

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_draw_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_draw_log_dom, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_draw_log_dom, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_draw_log_dom, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_draw_log_dom, __VA_ARGS__)

#endif
