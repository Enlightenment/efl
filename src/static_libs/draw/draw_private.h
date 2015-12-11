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

/* 255 - alpha */
static inline int
alpha_inverse(uint32_t color)
{
   color = ~color;
   return A_VAL(&color);
}

extern RGBA_Comp_Func_Solid func_for_mode_solid[EFL_GFX_RENDER_OP_LAST];
extern RGBA_Comp_Func func_for_mode[EFL_GFX_RENDER_OP_LAST];

void efl_draw_sse2_init(void);
void efl_draw_neon_init(void);

#endif
