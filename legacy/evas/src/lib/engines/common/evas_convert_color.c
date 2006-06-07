#include "evas_common.h"


void
evas_common_convert_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
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

void
evas_common_convert_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
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
   if (!(max & d))
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

void
evas_common_convert_hsv_to_rgb_int(int h, int s, int v, int *r, int *g, int *b)
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

void
evas_common_convert_rgb_to_hsv_int(int r, int g, int b, int *h, int *s, int *v)
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
   if (!(max & d))
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
