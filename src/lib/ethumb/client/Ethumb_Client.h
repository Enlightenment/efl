#ifndef __ETHUMB_CLIENT_H__
#define __ETHUMB_CLIENT_H__ 1

#include <Ethumb.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ETHUMB_CLIENT_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ETHUMB_CLIENT_BUILD */
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

/**
 * @defgroup Ethumb_Client Ethumb Client
 * @ingroup Ethumb
 *
 * @{
 */

/**
 * @defgroup Ethumb_Client_Basics Ethumb Client Basics
 *
 * Functions that all users must know of to use Ethumb_Client.
 *
 * @{
 */

/**
 * @brief client handle.
 *
 * The client handle is created by ethumb_client_connect() and
 * destroyed by ethumb_client_disconnect(). The connection and
 * requests are asynchronous and callbacks should be used to report
 * both success and failure of calls.
 */
typedef struct _Ethumb_Client Ethumb_Client;

/**
 * @brief client exists request handle.
 *
 * The exists request handle is created by ethumb_client_thumb_exists(),
 * automatically destroyed when it end and cancelled when requested by
 * ethumb_client_thumb_exists_cancel().
 */
typedef struct _Ethumb_Exists Ethumb_Exists;

/**
 * @brief reports results of ethumb_client_connect()
 *
 * @param data extra context given to ethumb_client_connect().
 * @param client handle of the current connection to server.
 * @param success @c EINA_TRUE if connected or @c EINA_FALSE if it was
 *        not possible.
 */
typedef void (*Ethumb_Client_Connect_Cb)(void *data, Ethumb_Client *client, Eina_Bool success);

/**
 * @brief reports server connection ended.
 *
 * Functions of this type may be called if they are set with
 * ethumb_client_on_server_die_callback_set().
 *
 * @param data extra context given to ethumb_client_on_server_die_callback_set().
 * @param client handle of the current connection to server.
 */
typedef void (*Ethumb_Client_Die_Cb)(void *data, Ethumb_Client *client);

/**
 * @brief reports results of ethumb_client_generate().
 *
 * @param data extra context given to ethumb_client_generate().
 * @param client handle of the current connection to server.
 * @param id identifier returned by ethumb_client_generate().
 * @param file path set with ethumb_client_file_set().
 * @param key value set with ethumb_client_file_set() or @c NULL.
 * @param thumb_path where thumbnail was stored, either set with
 *        ethumb_client_thumb_path_set() or automatically calculated
 *        using parameters.
 * @param thumb_key key inside thumb_path where thumbnail was stored or @c NULL.
 * @param success @c EINA_TRUE if generated or @c EINA_FALSE on errors.
 */
typedef void (*Ethumb_Client_Generate_Cb)(void *data, Ethumb_Client *client, int id, const char *file, const char *key, const char *thumb_path, const char *thumb_key, Eina_Bool success);

/**
 * @brief report results of ethumb_client_thumb_exists().
 *
 * @param client handle of the current connection to server.
 * @param exists EINA_TRUE if the thumbnail exists.
 * @param data extra context given to ethumb_client_thumb_exists().
 *
 * During the execution of the callback the state of the @p client is
 * temporarily realy restored to what it was when the call to
 * ethumb_client_thumb_exists() was done.
 */
typedef void (*Ethumb_Client_Thumb_Exists_Cb)(void *data, Ethumb_Client *client, Ethumb_Exists *thread, Eina_Bool exists);

/**
 * @brief reports results of ethumb_client_generate_cancel()
 *
 * @param data extra context given to ethumb_client_generate_cancel()
 * @param client handle of the current connection to server.
 */
typedef void (*Ethumb_Client_Generate_Cancel_Cb)(void *data, Eina_Bool success);

EAPI int ethumb_client_init(void);
EAPI int ethumb_client_shutdown(void);

EAPI Ethumb_Client *ethumb_client_connect(Ethumb_Client_Connect_Cb connect_cb, const void *data, Eina_Free_Cb free_data);
EAPI void ethumb_client_disconnect(Ethumb_Client *client);
EAPI void ethumb_client_on_server_die_callback_set(Ethumb_Client *client, Ethumb_Client_Die_Cb server_die_cb, const void *data, Eina_Free_Cb free_data);

/**
 * @}
 */

/**
 * @defgroup Ethumb_Client_Setup Ethumb Client Fine Tune Setup
 *
 * How to fine tune thumbnail generation, setting size, aspect, orientation,
 * frames, quality and so on.
 *
 * @{
 */

EAPI void ethumb_client_fdo_set(Ethumb_Client *client, Ethumb_Thumb_FDO_Size s);
EAPI void ethumb_client_size_set(Ethumb_Client *client, int tw, int th);
EAPI void ethumb_client_size_get(const Ethumb_Client *client, int *tw, int *th);
EAPI void ethumb_client_format_set(Ethumb_Client *client, Ethumb_Thumb_Format f);
EAPI Ethumb_Thumb_Format ethumb_client_format_get(const Ethumb_Client *client);
EAPI void ethumb_client_aspect_set(Ethumb_Client *client, Ethumb_Thumb_Aspect a);
EAPI Ethumb_Thumb_Aspect ethumb_client_aspect_get(const Ethumb_Client *client);
EAPI void ethumb_client_orientation_set(Ethumb_Client *client, Ethumb_Thumb_Orientation o);
EAPI Ethumb_Thumb_Orientation ethumb_client_orientation_get(const Ethumb_Client *client);
EAPI void ethumb_client_crop_align_set(Ethumb_Client *client, float x, float y);
EAPI void ethumb_client_crop_align_get(const Ethumb_Client *client, float *x, float *y);
EAPI void ethumb_client_quality_set(Ethumb_Client *client, int quality);
EAPI int ethumb_client_quality_get(const Ethumb_Client *client);
EAPI void ethumb_client_compress_set(Ethumb_Client *client, int compress);
EAPI int ethumb_client_compress_get(const Ethumb_Client *client);
EAPI Eina_Bool ethumb_client_frame_set(Ethumb_Client *client, const char *file, const char *group, const char *swallow);
EAPI void ethumb_client_dir_path_set(Ethumb_Client *client, const char *path);
EAPI const char * ethumb_client_dir_path_get(const Ethumb_Client *client);
EAPI void ethumb_client_category_set(Ethumb_Client *client, const char *category);
EAPI const char * ethumb_client_category_get(const Ethumb_Client *client);
EAPI void ethumb_client_video_time_set(Ethumb_Client *client, float time);
EAPI void ethumb_client_video_start_set(Ethumb_Client *client, float start);
EAPI void ethumb_client_video_interval_set(Ethumb_Client *client, float interval);
EAPI void ethumb_client_video_ntimes_set(Ethumb_Client *client, unsigned int ntimes);
EAPI void ethumb_client_video_fps_set(Ethumb_Client *client, unsigned int fps);
EAPI void ethumb_client_document_page_set(Ethumb_Client *client, unsigned int page);

EAPI void ethumb_client_ethumb_setup(Ethumb_Client *client);

EAPI void ethumb_client_thumb_path_set(Ethumb_Client *client, const char *path, const char *key);
EAPI void ethumb_client_thumb_path_get(Ethumb_Client *client, const char **path, const char **key);
/**
 * @}
 */

/**
 * @addtogroup Ethumb_Client_Basics Ethumb Client Basics
 * @{
 */
EAPI Eina_Bool ethumb_client_file_set(Ethumb_Client *client, const char *path, const char *key);
EAPI void ethumb_client_file_get(Ethumb_Client *client, const char **path, const char **key);
EAPI void ethumb_client_file_free(Ethumb_Client *client);

EAPI Ethumb_Exists *ethumb_client_thumb_exists(Ethumb_Client *client, Ethumb_Client_Thumb_Exists_Cb exists_cb, const void *data);
EAPI void ethumb_client_thumb_exists_cancel(Ethumb_Exists *exists);
EAPI Eina_Bool ethumb_client_thumb_exists_check(Ethumb_Exists *exists);
EAPI int  ethumb_client_generate(Ethumb_Client *client, Ethumb_Client_Generate_Cb generated_cb, const void *data, Eina_Free_Cb free_data);
EAPI void ethumb_client_generate_cancel(Ethumb_Client *client, int id, Ethumb_Client_Generate_Cancel_Cb cancel_cb, const void *data, Eina_Free_Cb free_data);
EAPI void ethumb_client_generate_cancel_all(Ethumb_Client *client);

typedef void (*Ethumb_Client_Async_Done_Cb)(Ethumb_Client *ethumbd, const char *thumb_path, const char *thumb_key, void *data);
typedef void (*Ethumb_Client_Async_Error_Cb)(Ethumb_Client *ethumbd, void *data);

typedef struct _Ethumb_Client_Async Ethumb_Client_Async;

EAPI Ethumb_Client_Async *ethumb_client_thumb_async_get(Ethumb_Client *client,
							Ethumb_Client_Async_Done_Cb done,
							Ethumb_Client_Async_Error_Cb error,
							const void *data);
EAPI void ethumb_client_thumb_async_cancel(Ethumb_Client *client, Ethumb_Client_Async *request);
  /**
 * @}
 */

/**
 * @}
 */


#ifdef __cplusplus
}
#endif
#endif /* __ETHUMB_CLIENT_H__ */
