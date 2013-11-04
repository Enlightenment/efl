#ifndef __ETHUMB_H__
#define __ETHUMB_H__ 1

#include <Eina.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ETHUMB_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ETHUMB_BUILD */
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

#ifdef __cplusplus
extern "C" {
#endif

#define ETHUMB_VERSION_MAJOR EFL_VERSION_MAJOR
#define ETHUMB_VERSION_MINOR EFL_VERSION_MINOR
   
   typedef struct _Ethumb_Version
     {
        int major;
        int minor;
        int micro;
        int revision;
     } Ethumb_Version;

   EAPI extern Ethumb_Version *ethumb_version;

/**
 * @page ethumb_main Ethumb
 *
 * @date 2009 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref ethumb_main_intro
 * @li @ref ethumb_main_compiling
 * @li @ref ethumb_main_next_steps
 *
 * @section ethumb_main_intro Introduction
 *
 * Ethumb will use @ref Evas to generate thumbnail images of given
 * files. The API allows great customization of the generated files
 * and also helps compling to FreeDesktop.Org Thumbnail Specification
 * (http://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html)
 *
 * However, thumbnailing can be an expensive process that will impact
 * your application experience, blocking animations and user
 * interaction during the generation. Another problem is that one
 * should try to cache the thumbnails in a place that other
 * applications can benefit from the file.
 *
 * @ref Ethumb_Client exists to solve this. It will communicate with a
 * server using standard D-Bus protocol. The server will use @ref
 * Ethumb itself to generate the thumbnail images and cache them using
 * FreeDesktop.Org standard. It is recommended that most applications
 * use @ref Ethumb_Client instead of @ref Ethumb directly.
 *
 * @section ethumb_main_compiling How to compile
 *
 * Ethumb is a library your application links to. The procedure for
 * this is very simple. Note that usually you want the D-Bus client
 * library. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script
 * outputs. For example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags ethumb_client`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs ethumb_client`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section ethumb_main_next_steps Next Steps
 *
 * After you understood what Ethumb is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Ethumb_Client to generate thumbnails using a server
 *     (recommended).
 * @li @ref Ethumb to generate thumbnails in the local process.
 *
 */

/**
 * @defgroup Ethumb Ethumb
 *
 * @{
 */
/**
 * @defgroup Ethumb_Basics Ethumb Basics
 *
 * Functions that all users must know of to use Ethumb.
 *
 * @{
 */

/**
 * @brief thumbnailer handle.
 *
 * The handle is returned by ethumb_new() and destroyed by ethumb_free().
 */
typedef struct _Ethumb Ethumb;

/**
 * @brief reports results of ethumb_generate().
 *
 * @param data extra context given to ethumb_generate().
 * @param e handle of the current thumbnailer.
 * @param success @c EINA_TRUE if generated or @c EINA_FALSE on errors.
 */
typedef void (*Ethumb_Generate_Cb)(void *data, Ethumb *e, Eina_Bool success);

EAPI int ethumb_init(void);
EAPI int ethumb_shutdown(void);

EAPI Ethumb * ethumb_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;
EAPI void ethumb_free(Ethumb *e);

/**
 * @}
 */

/**
 * @defgroup Ethumb_Setup Ethumb Fine Tune Setup
 *
 * How to fine tune thumbnail generation, setting size, aspect,
 * frames, quality and so on.
 *
 * @{
 */

EAPI Eina_Bool    ethumb_frame_set(Ethumb *e, const char *theme_file, const char *group, const char *swallow) EINA_ARG_NONNULL(1);
EAPI void         ethumb_frame_get(const Ethumb *e, const char **theme_file, const char **group, const char **swallow) EINA_ARG_NONNULL(1);

EAPI void         ethumb_thumb_dir_path_set(Ethumb *e, const char *path) EINA_ARG_NONNULL(1);
EAPI const char  *ethumb_thumb_dir_path_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void         ethumb_thumb_category_set(Ethumb *e, const char *category) EINA_ARG_NONNULL(1);
EAPI const char  *ethumb_thumb_category_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void         ethumb_thumb_path_set(Ethumb *e, const char *path, const char *key) EINA_ARG_NONNULL(1);
EAPI void         ethumb_thumb_path_get(Ethumb *e, const char **path, const char **key) EINA_ARG_NONNULL(1);
EAPI void         ethumb_thumb_hash(Ethumb *e) EINA_ARG_NONNULL(1);
EAPI void         ethumb_thumb_hash_copy(Ethumb *dst, const Ethumb *src) EINA_ARG_NONNULL(1, 2);

typedef enum _Ethumb_Thumb_FDO_Size
{
  ETHUMB_THUMB_NORMAL, /**< 128x128 as defined by FreeDesktop.Org standard */
  ETHUMB_THUMB_LARGE   /**< 256x256 as defined by FreeDesktop.Org standard */
} Ethumb_Thumb_FDO_Size;

typedef enum _Ethumb_Thumb_Format
{
   ETHUMB_THUMB_FDO,   /**< PNG as defined by FreeDesktop.Org standard */
   ETHUMB_THUMB_JPEG,  /**< JPEGs are often smaller and faster to read/write */
   ETHUMB_THUMB_EET    /**< EFL's own storage system, supports key parameter */
} Ethumb_Thumb_Format;

typedef enum _Ethumb_Thumb_Aspect
{
  ETHUMB_THUMB_KEEP_ASPECT, /**< keep original proportion between width and height */
  ETHUMB_THUMB_IGNORE_ASPECT, /**< ignore aspect and foce it to match thumbnail's width and height */
  ETHUMB_THUMB_CROP /**< keep aspect but crop (cut) the largest dimension */
} Ethumb_Thumb_Aspect;

typedef enum _Ethumb_Thumb_Orientation
{
  ETHUMB_THUMB_ORIENT_NONE,     /**< keep orientation as pixel data is */
  ETHUMB_THUMB_ROTATE_90_CW,    /**< rotate 90° clockwise */
  ETHUMB_THUMB_ROTATE_180,      /**< rotate 180° */
  ETHUMB_THUMB_ROTATE_90_CCW,   /**< rotate 90° counter-clockwise */
  ETHUMB_THUMB_FLIP_HORIZONTAL, /**< flip horizontally */
  ETHUMB_THUMB_FLIP_VERTICAL,   /**< flip vertically */
  ETHUMB_THUMB_FLIP_TRANSPOSE,  /**< transpose */
  ETHUMB_THUMB_FLIP_TRANSVERSE, /**< transverse */
  ETHUMB_THUMB_ORIENT_ORIGINAL  /**< use orientation from metadata (EXIF-only currently) */
} Ethumb_Thumb_Orientation;

EAPI void ethumb_thumb_fdo_set(Ethumb *e, Ethumb_Thumb_FDO_Size s) EINA_ARG_NONNULL(1);

EAPI void ethumb_thumb_size_set(Ethumb *e, int tw, int th) EINA_ARG_NONNULL(1);
EAPI void ethumb_thumb_size_get(const Ethumb *e, int *tw, int *th) EINA_ARG_NONNULL(1);

EAPI void                     ethumb_thumb_format_set(Ethumb *e, Ethumb_Thumb_Format f) EINA_ARG_NONNULL(1);
EAPI Ethumb_Thumb_Format      ethumb_thumb_format_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void                     ethumb_thumb_aspect_set(Ethumb *e, Ethumb_Thumb_Aspect a) EINA_ARG_NONNULL(1);
EAPI Ethumb_Thumb_Aspect      ethumb_thumb_aspect_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void                     ethumb_thumb_orientation_set(Ethumb *e, Ethumb_Thumb_Orientation o) EINA_ARG_NONNULL(1);
EAPI Ethumb_Thumb_Orientation ethumb_thumb_orientation_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI void         ethumb_thumb_crop_align_set(Ethumb *e, float x, float y) EINA_ARG_NONNULL(1);
EAPI void         ethumb_thumb_crop_align_get(const Ethumb *e, float *x, float *y) EINA_ARG_NONNULL(1);

EAPI void         ethumb_thumb_quality_set(Ethumb *e, int quality) EINA_ARG_NONNULL(1);
EAPI int          ethumb_thumb_quality_get(const Ethumb *e) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI void         ethumb_thumb_compress_set(Ethumb *e, int compress) EINA_ARG_NONNULL(1);
EAPI int          ethumb_thumb_compress_get(const Ethumb *e) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI void         ethumb_video_start_set(Ethumb *e, float start) EINA_ARG_NONNULL(1);
EAPI float        ethumb_video_start_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
EAPI void         ethumb_video_time_set(Ethumb *e, float time) EINA_ARG_NONNULL(1);
EAPI float        ethumb_video_time_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
EAPI void         ethumb_video_interval_set(Ethumb *e, float interval) EINA_ARG_NONNULL(1);
EAPI float        ethumb_video_interval_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
EAPI void         ethumb_video_ntimes_set(Ethumb *e, unsigned int ntimes) EINA_ARG_NONNULL(1);
EAPI unsigned int ethumb_video_ntimes_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
EAPI void         ethumb_video_fps_set(Ethumb *e, unsigned int fps) EINA_ARG_NONNULL(1);
EAPI unsigned int ethumb_video_fps_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;


EAPI void         ethumb_document_page_set(Ethumb *e, unsigned int page) EINA_ARG_NONNULL(1);
EAPI unsigned int ethumb_document_page_get(const Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;
/**
 * @}
 */

/**
 * @addtogroup Ethumb_Basics Ethumb Basics
 * @{
 */
EAPI Eina_Bool ethumb_file_set(Ethumb *e, const char *path, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void      ethumb_file_get(const Ethumb *e, const char **path, const char **key) EINA_ARG_NONNULL(1);
EAPI void      ethumb_file_free(Ethumb *e) EINA_ARG_NONNULL(1);

EAPI Eina_Bool ethumb_generate(Ethumb *e, Ethumb_Generate_Cb finished_cb, const void *data, Eina_Free_Cb free_data) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool ethumb_exists(Ethumb *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

EAPI Ethumb *ethumb_dup(const Ethumb *e) EINA_ARG_NONNULL(1);
EAPI Eina_Bool ethumb_cmp(const Ethumb *e1, const Ethumb *e2) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;
EAPI int ethumb_hash(const void *key, int key_length) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;
EAPI int ethumb_key_cmp(const void *key1, int key1_length,
			const void *key2, int key2_length) EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT EINA_PURE;
EAPI unsigned int ethumb_length(const void *key) EINA_PURE EINA_WARN_UNUSED_RESULT;

  /**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __ETHUMB_H__ */
