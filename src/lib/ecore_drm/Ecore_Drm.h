#ifndef _ECORE_DRM_H
# define _ECORE_DRM_H

# ifdef EAPI
#  undef EAPI
# endif

#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else // ifdef BUILDING_DLL
#  define EAPI __declspec(dllimport)
# endif // ifdef BUILDING_DLL
#else // ifdef _MSC_VER
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else // if __GNUC__ >= 4
#   define EAPI
#  endif // if __GNUC__ >= 4
# else // ifdef __GNUC__
#  define EAPI
# endif // ifdef __GNUC__
#endif // ifdef _MSC_VER

typedef enum _Ecore_Drm_Op
{
   ECORE_DRM_OP_READ_FD_SET,
   ECORE_DRM_OP_WRITE_FD_SET,
   ECORE_DRM_OP_OPEN_FD,
   ECORE_DRM_OP_CLOSE_FD
} Ecore_Drm_Op;

typedef enum _Ecore_Drm_Op_Result 
{
   ECORE_DRM_OP_SUCCESS,
   ECORE_DRM_OP_FAILURE
} Ecore_Drm_Op_Result;

/* structure for message passing */
typedef struct _Ecore_Drm_Message 
{
   int opcode, size;
   void *data;
} Ecore_Drm_Message;

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm_Device Ecore_Drm_Device;

/**
 * @file
 * @brief Ecore functions for dealing with drm, virtual terminals
 * 
 * @defgroup Ecore_Drm_Group Ecore_Drm - Drm Integration
 * @ingroup Ecore
 * 
 * Ecore_Drm provides a wrapper and functions for using libdrm
 * 
 * @li @ref Ecore_Drm_Init_Group
 * @li @ref Ecore_Drm_Device_Group
 * 
 */

EAPI int ecore_drm_init(void);
EAPI int ecore_drm_shutdown(void);

EAPI Ecore_Drm_Device *ecore_drm_device_find(const char *name, const char *seat);
EAPI void ecore_drm_device_free(Ecore_Drm_Device *dev);

#endif
