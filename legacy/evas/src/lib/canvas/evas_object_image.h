#ifndef EVAS_OBJECT_IMAGE_H
#define EVAS_OBJECT_IMAGE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* private struct for image object internal data */
typedef struct _Evas_Object_Image      Evas_Object_Image;

struct _Evas_Object_Image
{
   DATA32            magic;

   struct {
      struct {
         Evas_Common_Transform  transform;
         int          spread;
         int          x, y, w, h;
      } fill;
      struct {
         short       w, h, stride;
      } image;
      struct {
         short         l, r, t, b;
         unsigned char center_fill;
      } border;

      const char    *file;
      const char    *key;
      int            cspace;

      unsigned char  smooth_scale : 1;
      unsigned char  has_alpha :1;
   } cur, prev;

   int               pixels_checked_out;
   int               load_error;
   Evas_List        *pixel_updates;

   struct {
      unsigned char  scale_down_by;
      double         dpi;
      short          w, h;
   } load_opts;

   struct {
      void            (*get_pixels) (void *data, Evas_Object *o);
      void             *get_pixels_data;
   } func;

   void             *engine_data;

   unsigned char     changed : 1;
   unsigned char     dirty_pixels : 1;
};

#endif
