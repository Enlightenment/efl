#ifndef ECTOR_UTIL_H
# define ECTOR_UTIL_H

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

#endif
