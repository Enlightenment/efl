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

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Eina.h>

#ifdef __cplusplus
extern "C" {
#endif

enum _Ethumb_Thumb_FDO_Size
{
   ETHUMB_THUMB_NORMAL,
   ETHUMB_THUMB_LARGE
};

typedef enum _Ethumb_Thumb_FDO_Size Ethumb_Thumb_FDO_Size;

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

typedef struct _Ethumb_Frame Ethumb_Frame;
typedef struct _Ethumb Ethumb;
typedef void (*ethumb_generate_callback_t)(Ethumb *e, Eina_Bool success, void *data);

EAPI int ethumb_init(void);
EAPI int ethumb_shutdown(void);

EAPI Ethumb * ethumb_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;
EAPI void ethumb_free(Ethumb *e);

EAPI void ethumb_thumb_fdo_set(Ethumb *e, Ethumb_Thumb_FDO_Size s) EINA_ARG_NONNULL(1);

EAPI void ethumb_thumb_size_set(Ethumb *e, int tw, int th) EINA_ARG_NONNULL(1);
EAPI void ethumb_thumb_size_get(const Ethumb *e, int *tw, int *th) EINA_ARG_NONNULL(1);

EAPI void ethumb_thumb_format_set(Ethumb *e, Ethumb_Thumb_Format f) EINA_ARG_NONNULL(1);
EAPI Ethumb_Thumb_Format ethumb_thumb_format_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void ethumb_thumb_aspect_set(Ethumb *e, Ethumb_Thumb_Aspect a) EINA_ARG_NONNULL(1);
EAPI Ethumb_Thumb_Aspect ethumb_thumb_aspect_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void ethumb_thumb_crop_align_set(Ethumb *e, float x, float y) EINA_ARG_NONNULL(1);
EAPI void ethumb_thumb_crop_align_get(const Ethumb *e, float *x, float *y) EINA_ARG_NONNULL(1);

EAPI void ethumb_thumb_quality_set(Ethumb *e, int quality) EINA_ARG_NONNULL(1);
EAPI int ethumb_thumb_quality_get(const Ethumb *e) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI void ethumb_thumb_compress_set(Ethumb *e, int compress) EINA_ARG_NONNULL(1);
EAPI int ethumb_thumb_compress_get(const Ethumb *e) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI Eina_Bool ethumb_frame_set(Ethumb *e, const char *theme_file, const char *group, const char *swallow) EINA_ARG_NONNULL(1);
EAPI void ethumb_frame_get(const Ethumb *e, const char **theme_file, const char **group, const char **swallow) EINA_ARG_NONNULL(1);

EAPI void ethumb_thumb_dir_path_set(Ethumb *e, const char *path) EINA_ARG_NONNULL(1);
EAPI const char * ethumb_thumb_dir_path_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void ethumb_thumb_category_set(Ethumb *e, const char *category) EINA_ARG_NONNULL(1);
EAPI const char * ethumb_thumb_category_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void ethumb_video_time_set(Ethumb *e, float time) EINA_ARG_NONNULL(1);
EAPI float ethumb_video_time_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void ethumb_document_page_set(Ethumb *e, int page) EINA_ARG_NONNULL(1);
EAPI int ethumb_document_page_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI Eina_Bool ethumb_file_set(Ethumb *e, const char *path, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void ethumb_file_get(const Ethumb *e, const char **path, const char **key) EINA_ARG_NONNULL(1);
EAPI void ethumb_file_free(Ethumb *e) EINA_ARG_NONNULL(1);
EAPI void ethumb_thumb_path_set(Ethumb *e, const char *path, const char *key) EINA_ARG_NONNULL(1);
EAPI void ethumb_thumb_path_get(Ethumb *e, const char **path, const char **key) EINA_ARG_NONNULL(1);
EAPI Eina_Bool ethumb_generate(Ethumb *e, ethumb_generate_callback_t finished_cb, void *data, void (*free_data)(void *)) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool ethumb_exists(Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

#ifdef __cplusplus
}
#endif
#endif /* __ETHUMB_H__ */
