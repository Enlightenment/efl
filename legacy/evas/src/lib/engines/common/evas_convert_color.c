/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_convert_color.h"


EAPI void
evas_common_convert_argb_premul(DATA32 *data, unsigned int len)
{
   DATA32  *de = data + len;

   while (data < de)
     {
	DATA32  a = 1 + (*data >> 24);

	*data = (*data & 0xff000000) +
	  (((((*data) >> 8) & 0xff) * a) & 0xff00) +
	  (((((*data) & 0x00ff00ff) * a) >> 8) & 0x00ff00ff);
	data++;
     }
}

EAPI void
evas_common_convert_argb_unpremul(DATA32 *data, unsigned int len)
{
   DATA32  *de = data + len;

   while (data < de)
     {
	DATA32  a = (*data >> 24);

	if ((a > 0) && (a < 255))
	   *data = ARGB_JOIN(a,
			     (R_VAL(data) * 255) / a,
			     (G_VAL(data) * 255) / a,
			     (B_VAL(data) * 255) / a);
	data++;
     }

}

EAPI void
evas_common_convert_color_argb_premul(int a, int *r, int *g, int *b)
{
   a++;
   if (r) { *r = (a * *r) >> 8; }
   if (g) { *g = (a * *g) >> 8; }
   if (b) { *b = (a * *b) >> 8; }
}

EAPI void
evas_common_convert_color_argb_unpremul(int a, int *r, int *g, int *b)
{
   if (!a) return;
   if (r) { *r = (255 * *r) / a; }
   if (g) { *g = (255 * *g) / a; }
   if (b) { *b = (255 * *b) / a; }
}

EAPI void
evas_common_convert_color_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
{
   int i;
   float f;

   v *= 255;
   if (s == 0)
     {
       if (r) *r = v;
       if (g) *g = v;
       if (b) *b = v;
       return;
     }

   h /= 60;
   i = h;
   f = h - i;

   s *= v;
   f *= s;
   s = v - s;

   switch (i)
     {
       case 1:
         if (r) *r = v - f;  if (g) *g = v;  if (b) *b = s;
         return;
       case 2:
         if (r) *r = s;  if (g) *g = v;  if (b) *b = s + f;
         return;
       case 3:
         if (r) *r = s;  if (g) *g = v - f;  if (b) *b = v;
         return;
       case 4:
         if (r) *r = s + f;  if (g) *g = s;  if (b) *b = v;
         return;
       case 5:
         if (r) *r = v;  if (g) *g = s;  if (b) *b = v - f;
         return;
       default:
         if (r) *r = v;  if (g) *g = s + f;  if (b) *b = s;
         break;
     }
}

EAPI void
evas_common_convert_color_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
{
   int max, min, d = r - g;

   //set min to MIN(g,r)
   d = (d & (~(d >> 8)));
   min = r - d;
   //set max to MAX(g,r)
   max = g + d;

   //set min to MIN(b,min)
   d = min - b;
   min -= (d & (~(d >> 8)));

   //set max to MAX(max,b)
   d = b - max;
   max += (d & (~(d >> 8)));

   d = max - min;

   if (v) *v = (max / 255.0);
   if (!max)
     {
	if (s) *s = 0;
        if (h) *h = 0;
	return;
     }

   if (s) *s = (d / (float)max);
   if (r == max)
     {
       if (h)
         {
           *h = 60 * ((g - b) / (float)d);
           if (*h < 0) *h += 360;
         }
       return;
     }
   if (g == max)
     {
       if (h)
         {
           *h = 120 + (60 * ((b - r) / (float)d));
           if (*h < 0) *h += 360;
         }
       return;
     }
   if (h)
     {
       *h = 240 + (60 * ((r - g) / (float)d));
       if (*h < 0) *h += 360;
     }
}

EAPI void
evas_common_convert_color_hsv_to_rgb_int(int h, int s, int v, int *r, int *g, int *b)
{
   int   i, f;

   if (!s)
     {
	*r = *g = *b = v;
	return;
     }

   i = h / 255;
   f = h - (i * 255);
   s = (v * s) / 255;
   f = (s * f) / 255;
   s = v - s;

   switch (i)
     {
	case 1:
	  *r = v - f; *g = v; *b = s;
	  return;
	case 2:
	  *r = s; *g = v; *b = s + f;
	  return;
	case 3:
	  *r = s; *g = v - f; *b = v;
	  return;
	case 4:
	  *r = s + f; *g = s; *b = v;
	  return;
	case 5:
	  *r = v; *g = s; *b = v - f;
	  return;
	default:
	  *r = v; *g = s + f; *b = s;
         break;
     }
}

EAPI void
evas_common_convert_color_rgb_to_hsv_int(int r, int g, int b, int *h, int *s, int *v)
{
   int  min, max, d = r - g;

   d = (d & (~(d >> 8)));
   min = r - d;
   max = g + d;

   d = min - b;
   min -= (d & (~(d >> 8)));

   d = b - max;
   max += (d & (~(d >> 8)));

   d = max - min;

   *v = max;
   if (!max)
     {
	*s = *h = 0;
	return;
     }

   *s = ((d * 255) / max);

   if (r == max)
     {
	*h = (((g - b) * 255) / d);
	if (*h < 0) *h += 1530;
	return;
     }
   if (g == max)
     {
	*h = 510 + (((b - r) * 255) / d);
	if (*h < 0) *h += 1530;
	return;
     }
   *h = 1020 + (((r - g) * 255) / d);
   if (*h < 0) *h += 1530;

}
