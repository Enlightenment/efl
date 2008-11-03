#ifndef EVAS_OBJECT_GRADIENT2_H
#define EVAS_OBJECT_GRADIENT2_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* private struct for base gradient object internal data */
typedef struct _Evas_Object_Gradient2      Evas_Object_Gradient2;

struct _Evas_Object_Gradient2
{
   DATA32            magic;

   void             *engine_data;

   struct {
      struct {
         Evas_Common_Transform  transform;
         int         spread;
      } fill;
      unsigned char    gradient_opaque : 1;
   } cur, prev;

   unsigned char     gradient_changed : 1;
};


/* private struct for linear gradient object internal data */
typedef struct _Evas_Object_Gradient2_Linear      Evas_Object_Gradient2_Linear;

struct _Evas_Object_Gradient2_Linear
{
   Evas_Object_Gradient2 base;

   DATA32            magic;

   struct {
      struct {
          float  x0, y0, x1, y1;
      } fill;
   } cur, prev;

   unsigned char     gradient_changed : 1;
   unsigned char     changed : 1;
};

/* private struct for radial gradient object internal data */
typedef struct _Evas_Object_Gradient2_Radial      Evas_Object_Gradient2_Radial;

struct _Evas_Object_Gradient2_Radial
{
   Evas_Object_Gradient2 base;

   DATA32            magic;

   struct {
      struct {
         float  cx, cy, rx, ry;
      } fill;
   } cur, prev;

   unsigned char     gradient_changed : 1;
   unsigned char     changed : 1;
};

#endif
