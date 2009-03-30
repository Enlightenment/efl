#ifndef __ETHUMB_H__
#define __ETHUMB_H__ 1

#ifndef EAPI
#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#   define GNUC_NULL_TERMINATED
#  else
#   define EAPI
#   define GNUC_NULL_TERMINATED
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
#  define GNUC_NULL_TERMINATED
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#   define GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#  else
#   define EAPI
#  define GNUC_NULL_TERMINATED
#  endif
# else
#  define EAPI
#  define GNUC_NULL_TERMINATED
# endif
#endif /* ! _WIN32 */
#endif /* EAPI */

#include <Ecore_Evas.h>
#include <Evas.h>

#ifdef __cplusplus
extern "C" {
#endif

enum _Ethumb_Thumb_Size
{
   ETHUMB_THUMB_NORMAL,
   ETHUMB_THUMB_LARGE
};

typedef enum _Ethumb_Thumb_Size Ethumb_Thumb_Size;

enum _Ethumb_Thumb_Format
{
   ETHUMB_THUMB_FDO,
   ETHUMB_THUMB_JPEG
};

typedef enum _Ethumb_Thumb_Format Ethumb_Thumb_Format;

enum _Ethumb_Thumb_Aspect
{
   ETHUMB_THUMB_KEEP_ASPECT,
   ETHUMB_THUMB_IGNORE_ASPECT,
   ETHUMB_THUMB_CROP
};

typedef enum _Ethumb_Thumb_Aspect Ethumb_Thumb_Aspect;

struct _Ethumb_Frame
{
   const char *file;
   const char *group;
   const char *swallow;
   Evas_Object *edje;
};

typedef struct _Ethumb_Frame Ethumb_Frame;

struct _Ethumb
{
   const char *thumb_dir;
   const char *category;
   int tw, th;
   int format;
   int aspect;
   float crop_x, crop_y;
   Ethumb_Frame *frame;
   Ecore_Evas *ee, *sub_ee;
   Evas *e, *sub_e;
   Evas_Object *o, *img;
};

typedef struct _Ethumb Ethumb;

struct _Ethumb_File
{
   Ethumb *ethumb;
   const char *src_path;
   const char *thumb_path;
   int w, h;
};

typedef struct _Ethumb_File Ethumb_File;


EAPI int ethumb_init(void);
EAPI int ethumb_shutdown(void);

EAPI Ethumb * ethumb_new(void);
EAPI void ethumb_free(Ethumb *e);

EAPI void ethumb_thumb_fdo_set(Ethumb *e, Ethumb_Thumb_Size s);

EAPI void ethumb_thumb_size_set(Ethumb *e, int tw, int th);
EAPI void ethumb_thumb_size_get(const Ethumb *e, int *tw, int *th);

EAPI void ethumb_thumb_format_set(Ethumb *e, Ethumb_Thumb_Format f);
EAPI Ethumb_Thumb_Format ethumb_thumb_format_get(const Ethumb *e);

EAPI void ethumb_thumb_aspect_set(Ethumb *e, Ethumb_Thumb_Aspect a);
EAPI Ethumb_Thumb_Aspect ethumb_thumb_aspect_get(const Ethumb *e);

EAPI void ethumb_thumb_crop_align_set(Ethumb *e, float x, float y);
EAPI void ethumb_thumb_crop_align_get(Ethumb *e, float *x, float *y);

EAPI int ethumb_frame_set(Ethumb *e, const char *theme_file, const char *group, const char *swallow);

EAPI void ethumb_thumb_dir_path_set(Ethumb *e, const char *path);
EAPI const char * ethumb_thumb_dir_path_get(Ethumb *e);

EAPI void ethumb_thumb_category_set(Ethumb *e, const char *category);
EAPI const char * ethumb_thumb_category_get(Ethumb *e);

EAPI Ethumb_File * ethumb_file_new(Ethumb *e, const char *path);
EAPI void ethumb_file_free(Ethumb_File *ef);
EAPI void ethumb_file_thumb_path_set(Ethumb_File *ef, const char *path);
EAPI const char * ethumb_file_thumb_path_get(Ethumb_File *ef);
EAPI int ethumb_file_generate(Ethumb_File *ef);

#ifdef __cplusplus
}
#endif
#endif /* __ETHUMB_H__ */
