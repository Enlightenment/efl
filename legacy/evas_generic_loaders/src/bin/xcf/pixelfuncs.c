/*
  
  -----------------------------[ XCF Loader ]-----------------------------

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "common.h"

#define RS R_VAL(src + s_idx)
#define GS G_VAL(src + s_idx)
#define BS B_VAL(src + s_idx)
#define AS A_VAL(src + s_idx)
#define RD R_VAL(dest + d_idx)
#define GD G_VAL(dest + d_idx)
#define BD B_VAL(dest + d_idx)
#define AD A_VAL(dest + d_idx)

#define EPS 0.00001
#define PI  3.141592654
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))
#define LINEAR(x,y,w) ((w*y + x)*4)

void
rgb_to_hls (DATA8 *red, DATA8 *green, DATA8 *blue)
{
  int    r, g, b;
  double h, l, s;
  int    min, max;
  int    delta;

  r = *red;
  g = *green;
  b = *blue;

  if (r > g)
    {
      max = MAX (r, b);
      min = MIN (g, b);
    }
  else
    {
      max = MAX (g, b);
      min = MIN (r, b);
    }

  l = (max + min) / 2.0;

  if (max == min)
    {
      s = 0.0;
      h = 0.0;
    }
  else
    {
      delta = (max - min);

      if (l < 128)
	s = 255 * (double) delta / (double) (max + min);
      else
	s = 255 * (double) delta / (double) (511 - max - min);

      if (r == max)
	h = (g - b) / (double) delta;
      else if (g == max)
	h = 2 + (b - r) / (double) delta;
      else
	h = 4 + (r - g) / (double) delta;

      h = h * 42.5;

      if (h < 0)
	h += 255;
      else if (h > 255)
	h -= 255;
    }

  *red   = h;
  *green = l;
  *blue  = s;
}


DATA8
gimp_hls_value (double n1, double n2, double hue)
{
  double value;
  
  if (hue > 255)
    hue -= 255;
  else if (hue < 0)
    hue += 255;
  if (hue < 42.5)
    value = n1 + (n2 - n1) * (hue / 42.5);
  else if (hue < 127.5)
    value = n2;
  else if (hue < 170)
    value = n1 + (n2 - n1) * ((170 - hue) / 42.5);
  else
    value = n1;

  return (DATA8) (value * 255);
}


void
hls_to_rgb (DATA8 *hue, DATA8 *lightness, DATA8 *saturation)
{
  double h, l, s;
  double m1, m2;

  h = *hue;
  l = *lightness;
  s = *saturation;

  if (s == 0)
    {
      /*  achromatic case  */
      *hue        = l;
      *lightness  = l;
      *saturation = l;
    }
  else
    {
      if (l < 128)
	m2 = (l * (255 + s)) / 65025.0;
      else
	m2 = (l + s - (l * s) / 255.0) / 255.0;

      m1 = (l / 127.5) - m2;

      /*  chromatic case  */
      *hue        = gimp_hls_value (m1, m2, h + 85);
      *lightness  = gimp_hls_value (m1, m2, h);
      *saturation = gimp_hls_value (m1, m2, h - 85);
    }
}


void
rgb_to_hsv (DATA8 *red, DATA8 *green, DATA8 *blue)
{
  int    r, g, b;
  double h, s, v;
  int    min, max;
  int    delta;

  h = 0.0;

  r = *red;
  g = *green;
  b = *blue;

  if (r > g)
    {
      max = MAX (r, b);
      min = MIN (g, b);
    }
  else
    {
      max = MAX (g, b);
      min = MIN (r, b);
    }

  v = max;

  if (max != 0)
    s = ((max - min) * 255) / (double) max;
  else
    s = 0;

  if (s == 0)
    h = 0;
  else
    {
      delta = max - min;
      if (r == max)
	h = (g - b) / (double) delta;
      else if (g == max)
	h = 2 + (b - r) / (double) delta;
      else if (b == max)
	h = 4 + (r - g) / (double) delta;
      h *= 42.5;

      if (h < 0)
	h += 255;
      if (h > 255)
	h -= 255;
    }

  *red   = h;
  *green = s;
  *blue  = v;
}

void
hsv_to_rgb (DATA8 *hue, DATA8 *saturation, DATA8 *value)
{
  double h, s, v;
  double f, p, q, t;

  if (*saturation == 0)
    {
      *hue        = *value;
      *saturation = *value;
      *value      = *value;
    }
  else
    {
      h = *hue * 6.0  / 255.0;
      s = *saturation / 255.0;
      v = *value      / 255.0;

      f = h - (int) h;
      p = v * (1.0 - s);
      q = v * (1.0 - (s * f));
      t = v * (1.0 - (s * (1.0 - f)));

      switch ((int) h)
	{
	case 0:
	  *hue        = v * 255;
	  *saturation = t * 255;
	  *value      = p * 255;
	  break;

	case 1:
	  *hue        = q * 255;
	  *saturation = v * 255;
	  *value      = p * 255;
	  break;

	case 2:
	  *hue        = p * 255;
	  *saturation = v * 255;
	  *value      = t * 255;
	  break;

	case 3:
	  *hue        = p * 255;
	  *saturation = q * 255;
	  *value      = v * 255;
	  break;

	case 4:
	  *hue        = t * 255;
	  *saturation = p * 255;
	  *value      = v * 255;
	  break;

	case 5:
	  *hue        = v * 255;
	  *saturation = p * 255;
	  *value      = q * 255;
	  break;
	}
    }
}

/* translate negative destinations */
void _clip(int * src_tl_x, int * src_tl_y,
	  int * src_br_x, int * src_br_y,
	  int * dest_x, int * dest_y,
	  int dest_w, int dest_h)
{
  if (*dest_x + *src_br_x >= dest_w) 
    { *src_br_x -= (*dest_x + *src_br_x) - dest_w; }  

  if (*dest_y + *src_br_y >= dest_h) 
    { *src_br_y -= (*dest_y + *src_br_y) - dest_h; }  

  if (*dest_x < 0) 
    { 
      *src_tl_x = -(*dest_x); 
      *dest_x = 0; 
    } 
  if (*dest_y < 0) 
    { 
      *src_tl_y = -(*dest_y); 
      *dest_y = 0; 
    }
}

// FIXME: make sure layer alpha is used/applied in all cases
void
combine_pixels_normal (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  int b;
  unsigned char src_alpha;
  unsigned char new_alpha;
  float ratio, compl_ratio;
  long tmp;
 
  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	src_alpha = AS;
	
	if (src_alpha != 0)
	  {
	    if (src_alpha == 255)
	      new_alpha = src_alpha;
	    else
	      new_alpha = AD + INT_MULT((255 - AD), src_alpha, tmp);

	    b = 3;                                                                                  
	    if (new_alpha != 0)		       				                        
	      {											
		ratio = (float) src_alpha / new_alpha;						
		compl_ratio = 1.0 - ratio;							
		
		do
		  {
		    b--; 
		    dest[d_idx + b] =        								
		      (unsigned char) (src[s_idx + b] * ratio + dest[d_idx + b] * compl_ratio + EPS);   
		  }
		while (b); 
	      }

	    AD = new_alpha;	      
	  }
      }
}


void
combine_pixels_add (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;
  int tmp, tmp2;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
        tmp2 = INT_MULT(AS, RS, tmp);
	tmp = RD + tmp2;
	RD = (tmp > 255 ? 255 : tmp);

        tmp2 = INT_MULT(AS, GS, tmp);
	tmp = GD + tmp2;
	GD = (tmp > 255 ? 255 : tmp);

        tmp2 = INT_MULT(AS, BS, tmp);
	tmp = BD + tmp2;
	BD = (tmp > 255 ? 255 : tmp);

//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_sub (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;
  int tmp, tmp2;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
        tmp2 = INT_MULT(AS, RS, tmp);
	tmp = RD - tmp2;
	RD = (tmp < 0 ? 0 : tmp);

        tmp2 = INT_MULT(AS, GS, tmp);
	tmp = GD - tmp2;
	GD = (tmp < 0 ? 0 : tmp);

        tmp2 = INT_MULT(AS, BS, tmp);
	tmp = BD - tmp2;
	BD = (tmp < 0 ? 0 : tmp);

//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_diff (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;
  int tmp, tmp2;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
        tmp2 = INT_MULT(AS, RS, tmp);
	tmp = RD - tmp2;
	RD = (tmp < 0 ? -tmp : tmp);

        tmp2 = INT_MULT(AS, GS, tmp);
	tmp = GD - tmp2;
	GD = (tmp < 0 ? -tmp : tmp);

        tmp2 = INT_MULT(AS, BS, tmp);
	tmp = BD - tmp2;
	BD = (tmp < 0 ? -tmp : tmp);

//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_darken (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	RD = MIN(RD, RS);
	GD = MIN(GD, GS);
	BD = MIN(BD, BS);

//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_lighten (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	RD = MAX(RD, RS);
	GD = MAX(GD, GS);
	BD = MAX(BD, BS);

//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_mult (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;
  int tmp, tmp2, tmp3;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);

        tmp2 = INT_MULT(RS, AS, tmp);
        tmp2 = INT_MULT(RD, tmp2, tmp);
        tmp3 = INT_MULT(RD, (255 - AS), tmp);
        RD = (tmp2 + tmp3);
         
        tmp2 = INT_MULT(GS, AS, tmp);
        tmp2 = INT_MULT(GD, tmp2, tmp);
        tmp3 = INT_MULT(GD, (255 - AS), tmp);
        GD = (tmp2 + tmp3);

        tmp2 = INT_MULT(BS, AS, tmp);
        tmp2 = INT_MULT(BD, tmp2, tmp);
        tmp3 = INT_MULT(BD, (255 - AS), tmp);
        BD = (tmp2 + tmp3);

//	AS = MIN(AS, AD);
      }

//  combine_pixels_normal(src, src_w, src_h, dest, dest_w, dest_h, dest_x, dest_y);
}


void
combine_pixels_div (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	RS = MIN(255, ((float)RD / (RS + 1)) * 256);
	GS = MIN(255, ((float)GD / (GS + 1)) * 256);
	BS = MIN(255, ((float)BD / (BS + 1)) * 256);

//	AS = MIN(AD, AS);
      }

  combine_pixels_normal(src, src_w, src_h, dest, dest_w, dest_h, dest_x, dest_y);
}


void
combine_pixels_screen (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	RD = 255 - (((255 - RD) * (255 - RS)) >> 8);
	GD = 255 - (((255 - GD) * (255 - GS)) >> 8);
	BD = 255 - (((255 - BD) * (255 - BS)) >> 8);

//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_overlay (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;
  int tmp_screen, tmp_mult;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	tmp_screen = 255 - (((255 - RD) * (255 - RS)) >> 8);
	tmp_mult   = (RD * RS) >> 8;
	RD = (RD * tmp_screen + (255 - RD) * tmp_mult) >> 8;

	tmp_screen = 255 - (((255 - GD) * (255 - GS)) >> 8);
	tmp_mult   = (GD * GS) >> 8;
	GD = (GD * tmp_screen + (255 - GD) * tmp_mult) >> 8;

	tmp_screen = 255 - (((255 - BD) * (255 - BS)) >> 8);
	tmp_mult   = (BD * BS) >> 8;
	BD = (BD * tmp_screen + (255 - BD) * tmp_mult) >> 8;

//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_hsv (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y, int mode)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	rgb_to_hsv(&RS, &GS, &BS);
	rgb_to_hsv(&RD, &GD, &BD);

	switch (mode)
	  {
	  case 0: /* hue mode */
	    RD = RS;
	    break;
	  case 1: /* saturation mode */
	    GD = GS;
	    break;
	  case 2: /* value mode */
	    BD = BS;
	    break;
	  default:
	    break;
	  }
	
	hsv_to_rgb(&RD, &GD, &BD);
//	AD = MIN(AD, AS);	
      }
}


void
combine_pixels_hue (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  combine_pixels_hsv(src, src_w, src_h, dest, dest_w, dest_h, dest_x, dest_y, 0);
}


void
combine_pixels_sat (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  combine_pixels_hsv(src, src_w, src_h, dest, dest_w, dest_h, dest_x, dest_y, 1);
}


void
combine_pixels_val (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  combine_pixels_hsv(src, src_w, src_h, dest, dest_w, dest_h, dest_x, dest_y, 2);
}


void
combine_pixels_col (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);
  
  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);
	
	rgb_to_hls(&RS, &GS, &BS);
	rgb_to_hls(&RD, &GD, &BD);
	RD = RS;
	BD = BS;
	hls_to_rgb(&RD, &GD, &BD);
	
//	AD = MIN(AD, AS);
      }
}


void
combine_pixels_diss (DATA8* src, int src_w, int src_h, DATA8* dest, int dest_w, int dest_h, int dest_x, int dest_y)
{
  int x, y, s_idx, d_idx;
  int src_tl_x = 0, src_tl_y = 0;
  int src_br_x = src_w, src_br_y = src_h;

  srand(12345);

  _clip(&src_tl_x, &src_tl_y, &src_br_x, &src_br_y, &dest_x, &dest_y, dest_w, dest_h);

  for (y = src_tl_y; y < src_br_y; y++)
    for (x = src_tl_x; x < src_br_x; x++)
      {
	d_idx = LINEAR((dest_x + x - src_tl_x), (dest_y + y - src_tl_y), dest_w);
	s_idx = LINEAR(x, y, src_w);

	if ((rand() % 255) < AS)
	  {
	    int b;
	    unsigned char src_alpha;
	    unsigned char new_alpha;
	    float ratio, compl_ratio;
	    long tmp;
	    
	    src_alpha = AS;
	    
	    if (src_alpha != 0)
	      {
		if (src_alpha == 255)
		  new_alpha = src_alpha;
		else
		  new_alpha = AD + INT_MULT((255 - AD), src_alpha, tmp);
		
		b = 3;                                                                                  
		if (new_alpha != 0)		       				                        
		  {											
		    ratio = (float) src_alpha / new_alpha;						
		    compl_ratio = 1.0 - ratio;							
		    
		    do
		      {
			b--; 
			dest[d_idx + b] =        								
			  (unsigned char) (src[s_idx + b] * ratio + dest[d_idx + b] * compl_ratio + EPS);   
		      }
		    while (b); 
		  }
		
		AD = new_alpha;	      
	      }
	  }
      }
}
