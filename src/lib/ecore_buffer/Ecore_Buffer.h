#ifndef _ECORE_BUFFER_H_
# define _ECORE_BUFFER_H_

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

/**
 * @defgroup Ecore_Buffer_Group Ecore_Buffer - Graphics buffer functions
 * @ingroup Ecore
 *
 * The Ecore Buffer is an abstraction of graphic buffer.
 *
 * This library also provides simple mechanisms for sharing graphic buffer bet-
 * ween processes using wayland socket. Ecore Buffer Queue is for this
 * function, and it consists of two main object,
 * The Ecore_Buffer_Consumer and the Ecore_Buffer_Provider.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define __ecore_buffer_fourcc_code(a,b,c,d) ((unsigned int)(a) | ((unsigned int)(b) << 8) | \
                  ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24))

/* color index */
/**
 * @brief Definition for the Ecore_Buffer format C8 ([7:0] C).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_C8       __ecore_buffer_fourcc_code('C', '8', ' ', ' ')
/* 8 bpp RGB */
/**
 * @brief Definition for the Ecore_Buffer format RGB332 ([7:0] R:G:B 3:3:2).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGB332   __ecore_buffer_fourcc_code('R', 'G', 'B', '8')
/**
 * @brief Definition for the Ecore_Buffer format RGB233 ([7:0] B:G:R 2:3:3).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGR233   __ecore_buffer_fourcc_code('B', 'G', 'R', '8')
/* 16 bpp RGB */
/**
 * @brief Definition for the Ecore_Buffer format XRGB4444 ([15:0] x:R:G:B 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XRGB4444 __ecore_buffer_fourcc_code('X', 'R', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format XBRG4444 ([15:0] x:B:G:R 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XBGR4444 __ecore_buffer_fourcc_code('X', 'B', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format RGBX4444 ([15:0] R:G:B:x 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBX4444 __ecore_buffer_fourcc_code('R', 'X', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format BGRX4444 ([15:0] B:G:R:x 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRX4444 __ecore_buffer_fourcc_code('B', 'X', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format ARGB4444 ([15:0] A:R:G:B 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ARGB4444 __ecore_buffer_fourcc_code('A', 'R', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format ABGR4444 ([15:0] A:B:G:R 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ABGR4444 __ecore_buffer_fourcc_code('A', 'B', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format RGBA4444 ([15:0] R:G:B:A 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBA4444 __ecore_buffer_fourcc_code('R', 'A', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format BGRA4444 ([15:0] B:G:R:A 4:4:4:4 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRA4444 __ecore_buffer_fourcc_code('B', 'A', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format XRGB1555 ([15:0] x:R:G:B 1:5:5:5 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XRGB1555 __ecore_buffer_fourcc_code('X', 'R', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format XBGR1555 ([15:0] x:B:G:R 1:5:5:5 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XBGR1555 __ecore_buffer_fourcc_code('X', 'B', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format RGBX5551 ([15:0] R:G:B:x 5:5:5:1 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBX5551 __ecore_buffer_fourcc_code('R', 'X', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format BGRX5551 ([15:0] B:G:R:x 5:5:5:1 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRX5551 __ecore_buffer_fourcc_code('B', 'X', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format ARGB1555 ([15:0] A:R:G:B 1:5:5:5 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ARGB1555 __ecore_buffer_fourcc_code('A', 'R', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format ABGR1555 ([15:0] A:B:G:R 1:5:5:5 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ABGR1555 __ecore_buffer_fourcc_code('A', 'B', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format RGBA5551 ([15:0] R:G:B:A 5:5:5:1 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBA5551 __ecore_buffer_fourcc_code('R', 'A', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format BGRA5551 ([15:0] B:G:R:A 5:5:5:1 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRA5551 __ecore_buffer_fourcc_code('B', 'A', '1', '5')
/**
 * @brief Definition for the Ecore_Buffer format RGB565 ([15:0] R:G:B 5:6:5 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGB565   __ecore_buffer_fourcc_code('R', 'G', '1', '6')
/**
 * @brief Definition for the Ecore_Buffer format BGR565 ([15:0] B:G:R 5:6:5 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGR565   __ecore_buffer_fourcc_code('B', 'G', '1', '6')
/* 24 bpp RGB */
/**
 * @brief Definition for the Ecore_Buffer format RGB888 ([23:0] R:G:B little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGB888   __ecore_buffer_fourcc_code('R', 'G', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format BGR888 ([23:0] B:G:R little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGR888   __ecore_buffer_fourcc_code('B', 'G', '2', '4')
/* 32 bpp RGB */
/**
 * @brief Definition for the Ecore_Buffer format XRGB8888 ([31:0] x:R:G:B 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XRGB8888 __ecore_buffer_fourcc_code('X', 'R', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format XBGR8888 ([31:0] x:B:G:R 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XBGR8888 __ecore_buffer_fourcc_code('X', 'B', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format RGBX8888 ([31:0] R:G:B:x 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBX8888 __ecore_buffer_fourcc_code('R', 'X', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format BGRX8888 ([31:0] B:G:R:x 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRX8888 __ecore_buffer_fourcc_code('B', 'X', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format ARGB8888 ([31:0] A:R:G:B 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ARGB8888 __ecore_buffer_fourcc_code('A', 'R', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format ABGR8888 ([31:0] [31:0] A:B:G:R 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ABGR8888 __ecore_buffer_fourcc_code('A', 'B', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format RGBA8888 ([31:0] R:G:B:A 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBA8888 __ecore_buffer_fourcc_code('R', 'A', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format BGRA8888 ([31:0] B:G:R:A 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRA8888 __ecore_buffer_fourcc_code('B', 'A', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format XRGB2101010 ([31:0] x:R:G:B 2:10:10:10 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XRGB2101010  __ecore_buffer_fourcc_code('X', 'R', '3', '0')
/**
 * @brief Definition for the Ecore_Buffer format XBGR2101010 ([31:0] x:B:G:R 2:10:10:10 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_XBGR2101010  __ecore_buffer_fourcc_code('X', 'B', '3', '0')
/**
 * @brief Definition for the Ecore_Buffer format RGBX1010102 ([31:0] R:G:B:x 10:10:10:2 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBX1010102  __ecore_buffer_fourcc_code('R', 'X', '3', '0')
/**
 * @brief Definition for the Ecore_Buffer format BGRX1010102 ([31:0] B:G:R:x 10:10:10:2 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRX1010102  __ecore_buffer_fourcc_code('B', 'X', '3', '0')
/**
 * @brief Definition for the Ecore_Buffer format ARGB2101010 ([31:0] A:R:G:B 2:10:10:10 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ARGB2101010  __ecore_buffer_fourcc_code('A', 'R', '3', '0')
/**
 * @brief Definition for the Ecore_Buffer format ABGR2101010 ([31:0] A:B:G:R 2:10:10:10 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_ABGR2101010  __ecore_buffer_fourcc_code('A', 'B', '3', '0')
/**
 * @brief Definition for the Ecore_Buffer format RGBA1010102 ([31:0] R:G:B:A 10:10:10:2 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_RGBA1010102  __ecore_buffer_fourcc_code('R', 'A', '3', '0')
/**
 * @brief Definition for the Ecore_Buffer format BGRA1010102 ([31:0] B:G:R:A 10:10:10:2 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_BGRA1010102  __ecore_buffer_fourcc_code('B', 'A', '3', '0')
/* packed YCbCr */
/**
 * @brief Definition for the Ecore_Buffer format YUYV ([31:0] Cr0:Y1:Cb0:Y0 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YUYV     __ecore_buffer_fourcc_code('Y', 'U', 'Y', 'V')
/**
 * @brief Definition for the Ecore_Buffer format YVYU ([31:0] Cb0:Y1:Cr0:Y0 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YVYU     __ecore_buffer_fourcc_code('Y', 'V', 'Y', 'U')
/**
 * @brief Definition for the Ecore_Buffer format UYVY ([31:0] Y1:Cr0:Y0:Cb0 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_UYVY     __ecore_buffer_fourcc_code('U', 'Y', 'V', 'Y')
/**
 * @brief Definition for the Ecore_Buffer format VYUY ([31:0] Y1:Cb0:Y0:Cr0 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_VYUY     __ecore_buffer_fourcc_code('V', 'Y', 'U', 'Y')
/**
 * @brief Definition for the Ecore_Buffer format AYUV ([31:0] A:Y:Cb:Cr 8:8:8:8 little endian).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_AYUV     __ecore_buffer_fourcc_code('A', 'Y', 'U', 'V')
/*
 * 2 plane YCbCr
 * index 0 = Y plane, [7:0] Y
 * index 1 = Cr:Cb plane, [15:0] Cr:Cb little endian
 * or
 * index 1 = Cb:Cr plane, [15:0] Cb:Cr little endian
 */
/**
 * @brief Definition for the Ecore_Buffer format NV12 (2x2 subsampled Cr:Cb plane).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_NV12     __ecore_buffer_fourcc_code('N', 'V', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format NV21 (2x2 subsampled Cb:Cr plane).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_NV21     __ecore_buffer_fourcc_code('N', 'V', '2', '1')
/**
 * @brief Definition for the Ecore_Buffer format NV16 (2x1 subsampled Cr:Cb plane).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_NV16     __ecore_buffer_fourcc_code('N', 'V', '1', '6')
/**
 * @brief Definition for the Ecore_Buffer format NV61 (2x1 subsampled Cb:Cr plane).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_NV61     __ecore_buffer_fourcc_code('N', 'V', '6', '1')
/*
 * 3 plane YCbCr
 * index 0: Y plane, [7:0] Y
 * index 1: Cb plane, [7:0] Cb
 * index 2: Cr plane, [7:0] Cr
 * or
 * index 1: Cr plane, [7:0] Cr
 * index 2: Cb plane, [7:0] Cb
 */
/**
 * @brief Definition for the Ecore_Buffer format YUV410 (4x4 subsampled Cb (1) and Cr (2) planes).
 */
#define ECORE_BUFFER_FORMAT_YUV410   __ecore_buffer_fourcc_code('Y', 'U', 'V', '9')
/**
 * @brief Definition for the Ecore_Buffer format YVU410 (4x4 subsampled Cr (1) and Cb (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YVU410   __ecore_buffer_fourcc_code('Y', 'V', 'U', '9')
/**
 * @brief Definition for the Ecore_Buffer format YUV411 (4x1 subsampled Cb (1) and Cr (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YUV411   __ecore_buffer_fourcc_code('Y', 'U', '1', '1')
/**
 * @brief Definition for the Ecore_Buffer format YVU411 (4x1 subsampled Cr (1) and Cb (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YVU411   __ecore_buffer_fourcc_code('Y', 'V', '1', '1')
/**
 * @brief Definition for the Ecore_Buffer format YUV420 (2x2 subsampled Cb (1) and Cr (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YUV420   __ecore_buffer_fourcc_code('Y', 'U', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format YVU420 (2x2 subsampled Cr (1) and Cb (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YVU420   __ecore_buffer_fourcc_code('Y', 'V', '1', '2')
/**
 * @brief Definition for the Ecore_Buffer format YUV422 (2x1 subsampled Cb (1) and Cr (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YUV422   __ecore_buffer_fourcc_code('Y', 'U', '1', '6')
/**
 * @brief Definition for the Ecore_Buffer format YVU422 (2x1 subsampled Cr (1) and Cb (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YVU422   __ecore_buffer_fourcc_code('Y', 'V', '1', '6')
/**
 * @brief Definition for the Ecore_Buffer format YUV444 (non-subsampled Cb (1) and Cr (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YUV444   __ecore_buffer_fourcc_code('Y', 'U', '2', '4')
/**
 * @brief Definition for the Ecore_Buffer format YVU444 (non-subsampled Cr (1) and Cb (2) planes).
 * @since 1.15
 */
#define ECORE_BUFFER_FORMAT_YVU444   __ecore_buffer_fourcc_code('Y', 'V', '2', '4')

/**
 * @defgroup Ecore_Buffer_Lib_Group Ecore Buffer Library Functions
 * @ingroup Ecore_Buffer_Group
 *
 * Utility functions that set up and shut down the Ecore Buffer library.
 * This group of functions is applied to an Ecore_Buffer object.
 *
 * @{
 */

/**
 * @typedef Ecore_Buffer
 * An object representing a graphic buffer.
 * @since 1.15
 */
typedef struct _Ecore_Buffer Ecore_Buffer;
/**
 * @typedef Ecore_Buffer_Backend
 * The interfaces for backend of buffer.
 * @since 1.15
 */
typedef struct _Ecore_Buffer_Backend Ecore_Buffer_Backend;
/**
 * @typedef Ecore_Export_Type
 * Types for export buffer.
 * @since 1.15
 */
typedef enum _Ecore_Export_Type Ecore_Export_Type;
/**
 * @typedef Ecore_Buffer_Format
 * The format of Ecore_Buffer.
 * @since 1.15
 */
typedef unsigned int Ecore_Buffer_Format;
/**
 * @typedef Ecore_Pixmap
 * An Id of Pixmap.
 * @since 1.15
 */
typedef unsigned long Ecore_Pixmap;
/**
 * @typedef Ecore_Buffer_Module_Data
 * The data of module.
 * @since 1.15
 */
typedef void* Ecore_Buffer_Module_Data;
/**
 * @typedef Ecore_Buffer_Data
 * The data of Ecore_Buffer.
 * @since 1.15
 */
typedef void* Ecore_Buffer_Data;
/**
 * @typedef Ecore_Buffer_Cb
 * Called whenever Ecore_Buffer is freed.
 * @since 1.15
 */
typedef void (*Ecore_Buffer_Cb)(Ecore_Buffer* buf, void* data);

/**
 * @enum _Ecore_Export_Type
 * Types for export buffer.
 * @since 1.15
 */
enum _Ecore_Export_Type
{
   EXPORT_TYPE_INVALID,
   EXPORT_TYPE_ID,
   EXPORT_TYPE_FD
};

/**
 * @struct _Ecore_Buffer_Backend
 * @brief Structure used when initializing Ecore Buffer Backend. This structure
 * is mainly used by modules implementing the Ecore Buffer Backend interface.
 * @since 1.15
 */
struct _Ecore_Buffer_Backend
{
   const char *name; /**< The name of backend */

   Ecore_Buffer_Module_Data    (*init)(const char *context, const char *options); /**< Initialize the backend */
   void                        (*shutdown)(Ecore_Buffer_Module_Data bmdata); /**< Shut down the backend */
   Ecore_Buffer_Data           (*buffer_alloc)(Ecore_Buffer_Module_Data bmdata,
                                               int width, int height,
                                               Ecore_Buffer_Format format,
                                               unsigned int flags); /**< Newly allocate memory for buffer */
   void                        (*buffer_free)(Ecore_Buffer_Module_Data bmdata,
                                              Ecore_Buffer_Data bdata); /**< Free allocated memory */
   Ecore_Export_Type           (*buffer_export)(Ecore_Buffer_Module_Data bmdata,
                                                Ecore_Buffer_Data bdata, int *id); /**< Get the id or fd of Ecore_Buffer for exporting it */
   Ecore_Buffer_Data           (*buffer_import)(Ecore_Buffer_Module_Data bmdata,
                                                int w, int h,
                                                Ecore_Buffer_Format format,
                                                Ecore_Export_Type type,
                                                int export_id,
                                                unsigned int flags); /**< Import and create Ecore_Buffer from id or fd */
   void                       *(*data_get)(Ecore_Buffer_Module_Data bmdata,
                                           Ecore_Buffer_Data bdata);
   Ecore_Pixmap                (*pixmap_get)(Ecore_Buffer_Module_Data bmdata,
                                             Ecore_Buffer_Data bdata); /**< Get the pixmap handle */
   void                       *(*tbm_surface_get)(Ecore_Buffer_Module_Data bmdata,
                                                  Ecore_Buffer_Data bdata); /**< Get the tbm_surface_h handle */
};

/**
 * @brief Initializes the Ecore_Buffer system.
 *
 * @since 1.15
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @see ecore_buffer_shutdown()
 */
EAPI Eina_Bool     ecore_buffer_init(void);
/**
 * @brief Shuts down the Ecore_Buffer system.
 *
 * @since 1.15
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @see ecore_buffer_init()
 */
EAPI Eina_Bool     ecore_buffer_shutdown(void);
/**
 * @brief Registers the given buffer backend.
 *
 * @since 1.15
 *
 * @param[in] be The backend
 *
 * @return @c EINA_TRUE if backend has been correctly registered, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool     ecore_buffer_register(Ecore_Buffer_Backend *be);
/**
 * @brief Unregisters the given buffer backend.
 *
 * @since 1.15
 *
 * @param[in] be The backend
 */
EAPI void          ecore_buffer_unregister(Ecore_Buffer_Backend *be);
/**
 * @brief Creates a new Ecore_Buffer given type.
 *
 * @since 1.15
 *
 * @param[in] engine The name of backend
 * @param[in] width Width for Ecore_Buffer
 * @param[in] height Height for Ecore_Buffer
 * @param[in] format Format for Ecore_Buffer
 * @param[in] flags Flags for Ecore_Buffer
 *
 * @return Newly allocated Ecore_Buffer instance, NULL otherwise.
 */
EAPI Ecore_Buffer *ecore_buffer_new(const char *engine, unsigned int width, unsigned int height, Ecore_Buffer_Format format, unsigned int flags);
/**
 * @brief Frees the given Ecore_Buffer.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer to free
 */
EAPI void          ecore_buffer_free(Ecore_Buffer *buf);
/**
 * @brief Sets a callback for Ecore_Buffer free events.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer to set callbacks on
 * @param[in] func The function to call
 * @param[in] data A pointer to the user data to store.
 *
 * A call to this function will set a callback on an Ecore_Buffer, causing
 * @p func to be called whenever @p buf is freed.
 *
 * @see ecore_buffer_free_callback_remove()
 */
EAPI void          ecore_buffer_free_callback_add(Ecore_Buffer *buf, Ecore_Buffer_Cb  func, void *data);
/**
 * @brief Removes a callback for Ecore_Buffer free events.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer to remove callbacks on
 * @param[in] func The function to remove
 * @param[in] data A pointer to the user data to remove
 *
 * @see ecore_buffer_free_callback_add()
 */
EAPI void          ecore_buffer_free_callback_remove(Ecore_Buffer *buf, Ecore_Buffer_Cb func, void *data);
/**
 * @brief Get a pointer to the raw data of the given Ecore_Buffer.
 *
 * @param[in] buf The Ecore_Buffer.
 *
 * @return The pointer of raw data.
 */
EAPI void         *ecore_buffer_data_get(Ecore_Buffer *buf);
/**
 * @brief Returns the Pixmap of given Ecore_Buffer.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer
 *
 * @return The Pixmap instance, @c 0 otherwise.
 */
EAPI Ecore_Pixmap  ecore_buffer_pixmap_get(Ecore_Buffer *buf);
/**
 * @brief Returns the tbm surface handle of given Ecore_Buffer.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer
 *
 * @return The tbm surface handle, NULL otherwise.
 *
 * The tbm surface handle will be used for the API of libtbm.
 * The API is described in tbm_surface.h in libtbm.
 */
EAPI void         *ecore_buffer_tbm_surface_get(Ecore_Buffer *buf);
/**
 * @brief Returns size of given Ecore_Buffer.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer
 * @param[out] width Where to return the width value. May be @c NULL.
 * @param[out] height Where to return the height value. May be @c NULL.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool     ecore_buffer_size_get(Ecore_Buffer *buf, unsigned int *width, unsigned int *height);
/**
 * @brief Returns format of given Ecore_Buffer.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer
 *
 * @return The format of given Ecore_Buffer.
 *
 * Return value can be one of those pre-defined value such as ECORE_BUFFER_FORMAT_XRGB8888.
 */
EAPI Ecore_Buffer_Format ecore_buffer_format_get(Ecore_Buffer *buf);
/**
 * @brief Returns the flags of given Ecore_Buffer.
 *
 * @since 1.15
 *
 * @param[in] buf The Ecore_Buffer
 *
 * @return The flags of given Ecore_Buffer.
 *
 * NOTE: Not Defined yet.
 */
EAPI unsigned int  ecore_buffer_flags_get(Ecore_Buffer *buf);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
