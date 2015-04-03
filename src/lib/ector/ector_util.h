#ifndef ECTOR_UTIL_H
# define ECTOR_UTIL_H

static inline unsigned int
ector_color_multiply(unsigned int c1, unsigned int c2)
{
   return ( ((((((c1) >> 16) & 0xff00) * (((c2) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) +
            ((((((c1) >> 8) & 0xff00) * (((c2) >> 16) & 0xff)) + 0xff00) & 0xff0000) +
            ((((((c1) & 0xff00) * ((c2) & 0xff00)) + 0xff0000) >> 16) & 0xff00) +
            (((((c1) & 0xff) * ((c2) & 0xff)) + 0xff) >> 8) );
}

#endif
