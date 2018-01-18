#ifndef __ETHUMB_PRIVATE_H__
#define __ETHUMB_PRIVATE_H__ 1

#include <Ethumb.h>
#include <Ethumb_Plugin.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

typedef struct _Ethumb_Frame Ethumb_Frame;

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
   int orientation;
   float crop_x, crop_y;
   int quality;
   int compress;
   const char *src_hash;
   const char *src_path;
   const char *src_key;
   const char *thumb_path;
   const char *thumb_key;
   int rw, rh;
   struct
     {
	double start, time, interval;
	unsigned int ntimes, fps;
     } video;
   struct
     {
	unsigned int page;
     } document;
   Ethumb_Frame *frame;
   Ecore_Evas *ee, *sub_ee;
   Evas *e, *sub_e;
   Evas_Object *o, *img;
   Ecore_Idler *finished_idler;
   Ethumb_Generate_Cb finished_cb;
   void *cb_data;
   Eina_Free_Cb cb_data_free;
   int cb_result;

   void *pdata;
   Ethumb_Plugin *plugin;
};

#undef EAPI
#define EAPI

#endif /* __ETHUMB_PRIVATE_H__ */
