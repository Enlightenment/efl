#ifndef EVAS_GRADIENT2_H_
#define EVAS_GRADIENT2_H_


typedef struct _Evas_Object_Gradient2      Evas_Object_Gradient2;

struct _Evas_Object_Gradient2
{
   DATA32            magic;

   struct {
      struct {
         Evas_Common_Transform  transform;
         int         spread;
      } fill;
      unsigned char    gradient_opaque : 1;
   } cur, prev;

   unsigned char     gradient_changed : 1;
};


#endif
