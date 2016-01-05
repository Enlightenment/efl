#ifndef _EVAS_NATIVE_COMMON_H
#define _EVAS_NATIVE_COMMON_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
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
#endif /* ! _WIN32 */

//#include <Evas_Common.h>

typedef struct _Native Native;
struct _Native
{
   Evas_Native_Surface ns;
   union {
      /*  EVAS_NATIVE_SURFACE_X11 */
      struct
      {
         unsigned long     pixmap; /* Pixmap */
         void                     *visual;  /* Visual */
         void                     *display; /* Display */
         void                     *exim;    /* Ecore_X_Image or Evas_DRI_Image */

         void                     *buffer;
         void                     *config;   /* egl configuration or glx configuration */
         void                     *surface; /* egl surface or glx surface */
      } x11; /**< Set this struct fields if surface data is SW X11 based. */

       /*  EVAS_NATIVE_SURFACE_WL */
      struct
      {
         void                     *wl_buf; /* struct wl_buffer */
         void                     *surface; /*egl surface*/
      } wl_surface; /**< Set this struct fields if surface data is Wayland based. */

      /* EVAS_NATIVE_SURFACE_OPENGL */
      struct
      {
         void                     *surface; /*egl surface*/
      } opengl;

      /* EVAS_NATIVE_SURFACE_EVASGL */
      struct
      {
         void                     *surface; /*evas gl surface*/
      } evasgl;

      /*  EVAS_NATIVE_SURFACE_TBM */
      struct
      {
         void                     *buffer; /*tbm surface*/
         void                     *surface; /*egl surface*/
      } tbm;
   } ns_data; /**< Choose one union data according to your surface in Evas Engine. */
};

EAPI void * evas_native_tbm_surface_image_set(void *data, void *image, void *native);

typedef void *(*Evas_Native_Tbm_Surface_Image_Set_Call)(void *data, void *image, void *native);

#endif //_EVAS_NATIVE_COMMON_H
