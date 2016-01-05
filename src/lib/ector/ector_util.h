#ifndef ECTOR_UTIL_H
# define ECTOR_UTIL_H

#define ECTOR_COLOR_SET(value) (value << 8)

#define ECTOR_COLOR16_SET(value) (value)

typedef struct _Ector_Color Ector_Color;

typedef enum _Ector_Color_Type
  {
    ECTOR_COLOR,
    ECTOR_COLOR16
  } Ector_Color_Type;

struct _Ector_Color
{
   unsigned short r;
   unsigned short g;
   unsigned short b;
   unsigned short a;

   Ector_Color_Type type;
};

static inline void
ector_color_argb_premul(int a, int *r, int *g, int *b)
{
   a++;
   if (r) { *r = (a * *r) >> 8; }
   if (g) { *g = (a * *g) >> 8; }
   if (b) { *b = (a * *b) >> 8; }
}

static inline void
ector_color_argb_unpremul(int a, int *r, int *g, int *b)
{
   if (!a) return;
   if (r) { *r = (255 * *r) / a; }
   if (g) { *g = (255 * *g) / a; }
   if (b) { *b = (255 * *b) / a; }
}


static inline unsigned int
ector_color_multiply(unsigned int c1, unsigned int c2)
{
   return ( ((((((c1) >> 16) & 0xff00) * (((c2) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) +
            ((((((c1) >> 8) & 0xff00) * (((c2) >> 16) & 0xff)) + 0xff00) & 0xff0000) +
            ((((((c1) & 0xff00) * ((c2) & 0xff00)) + 0xff0000) >> 16) & 0xff00) +
            (((((c1) & 0xff) * ((c2) & 0xff)) + 0xff) >> 8) );
}

static inline void
ector_color_set(Ector_Color *color, unsigned char r, unsigned char g,
                                    unsigned char b, unsigned char a)
{
   color->r = ECTOR_COLOR_SET(r);
   color->g = ECTOR_COLOR_SET(g);
   color->b = ECTOR_COLOR_SET(b);
   color->a = ECTOR_COLOR_SET(a);

   color->type = ECTOR_COLOR;
}

static inline void
ector_color16_set(Ector_Color *color, unsigned short r, unsigned short g,
                                      unsigned short b, unsigned short a)
{
   color->r = ECTOR_COLOR16_SET(r);
   color->g = ECTOR_COLOR16_SET(g);
   color->b = ECTOR_COLOR16_SET(b);
   color->a = ECTOR_COLOR16_SET(a);

   color->type = ECTOR_COLOR16;
}

static inline Ector_Color_Type
ector_color_type_get(Ector_Color *color)
{
   return color->type;
}

#endif
