#ifndef _EVAS_NATIVE_COMMON_H
#define _EVAS_NATIVE_COMMON_H

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

//#include <Evas_Common.h>

#define EVAS_DMABUF_ATTRIBUTE_VERSION 1

struct dmabuf_attributes
{
  /* This must exactly match the struct in Enlightenment.
   * Wayland dmabuf is still an experimental protocol and may
   * change.  If the version doesn't match we won't even attempt
   * to read the struct.
   */
   int version;
   int32_t width;
   int32_t height;
   uint32_t format;
   uint32_t flags; /* enum zlinux_buffer_params_flags */
   int n_planes;
   int fd[4];
   uint32_t offset[4];
   uint32_t stride[4];
   uint64_t modifier[4];
};

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
         unsigned char             multiple_buffer : 1; /* whether pixmap is multiple buffer */
      } x11; /**< Set this struct fields if surface data is SW X11 based. */

       /*  EVAS_NATIVE_SURFACE_WL */
      struct
      {
         void                     *wl_buf; /* struct wl_buffer */
         void                     *surface; /*egl surface*/
      } wl_surface; /**< Set this struct fields if surface data is Wayland based. */

       /*  EVAS_NATIVE_SURFACE_WL_DMABUF */
      struct
      {
         struct dmabuf_attributes attr; /* Plane attributes of buffer */
         void                     *resource; /* Wayland resource for buffer */

         // Run-time storage for bind/unbind
         size_t                   size; /* size of are when mmapped */
         void                     *ptr; /* data area when mmapped */
         void                     *image; /* EGLImage when bound for GL */
      } wl_surface_dmabuf; /**< Set this struct fields if surface data is Wayland dmabuf based. */

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

EAPI void *_evas_native_tbm_surface_image_set(void *data, void *image, void *native);
EAPI int _evas_native_tbm_surface_stride_get(void *data, void *native);
EAPI int _evas_native_tbm_init(void);
EAPI void _evas_native_tbm_shutdown(void);

void *_evas_native_dmabuf_surface_image_set(void *image, void *native);

typedef void *(*Evas_Native_Tbm_Surface_Image_Set_Call)(void *data, void *image, void *native);
typedef int (*Evas_Native_Tbm_Surface_Stride_Get_Call)(void *data, void *native);

#endif //_EVAS_NATIVE_COMMON_H
