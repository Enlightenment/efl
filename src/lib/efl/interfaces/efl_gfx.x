#ifndef EFL_GFX_X
# define EFL_GFX_X

static inline void
efl_gfx_color_type_set(Efl_Gfx_Color *color,
                       unsigned char r, unsigned char g,
                       unsigned char b, unsigned char a)
{
   color->r = EFL_GFX_COLOR_SET(r);
   color->g = EFL_GFX_COLOR_SET(g);
   color->b = EFL_GFX_COLOR_SET(b);
   color->a = EFL_GFX_COLOR_SET(a);

   color->type = EFL_GFX_COLOR_TYPE_BITS8;
}

static inline void
efl_gfx_color16_type_set(Efl_Gfx_Color *color,
                         unsigned short r, unsigned short g,
                         unsigned short b, unsigned short a)
{
   color->r = EFL_GFX_COLOR16_SET(r);
   color->g = EFL_GFX_COLOR16_SET(g);
   color->b = EFL_GFX_COLOR16_SET(b);
   color->a = EFL_GFX_COLOR16_SET(a);

   color->type = EFL_GFX_COLOR_TYPE_BITS16;
}

static inline Efl_Gfx_Color_Type
efl_gfx_color_type_get(Efl_Gfx_Color *color)
{
   return color->type;
}

#endif
