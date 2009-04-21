#ifndef __ETHUMB_PRIVATE_H__
#define __ETHUMB_PRIVATE_H__ 1

#include <Ethumb.h>

struct _Ethumb_Frame
{
   const char *file;
   const char *group;
   const char *swallow;
   Evas_Object *edje;
};

struct _Ethumb
{
   const char *thumb_dir;
   const char *category;
   int tw, th;
   int format;
   int aspect;
   float crop_x, crop_y;
   struct
     {
	double time;
     } video;
   struct
     {
	int page;
     } document;
   Ethumb_Frame *frame;
   Ecore_Evas *ee, *sub_ee;
   Evas *e, *sub_e;
   Evas_Object *o, *img;
   Ecore_Idler *finished_idler;
   ethumb_generate_callback_t finished_cb;
   void *cb_data;
};

struct _Ethumb_File
{
   Ethumb *ethumb;
   const char *src_path;
   const char *src_key;
   const char *thumb_path;
   const char *thumb_key;
   int w, h;
};

#endif /* __ETHUMB_PRIVATE_H__ */
