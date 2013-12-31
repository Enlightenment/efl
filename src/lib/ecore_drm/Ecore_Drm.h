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
   ECORE_DRM_OP_DEVICE_OPEN,
   ECORE_DRM_OP_DEVICE_CLOSE,
   ECORE_DRM_OP_DEVICE_MASTER_DROP,
   ECORE_DRM_OP_DEVICE_MASTER_SET,
   ECORE_DRM_OP_TTY_OPEN,
   ECORE_DRM_OP_TTY_CLOSE
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

/* opaque structure to represent a drm output mode */
typedef struct _Ecore_Drm_Output_Mode Ecore_Drm_Output_Mode;

/* opaque structure to represent a drm output */
typedef struct _Ecore_Drm_Output Ecore_Drm_Output;

/* opaque structure to represent a drm udev input */
typedef struct _Ecore_Drm_Input Ecore_Drm_Input;

/* opaque structure to represent a drm evdev input */
typedef struct _Ecore_Drm_Evdev Ecore_Drm_Evdev;

/* opaque structure to represent a drm seat */
typedef struct _Ecore_Drm_Seat Ecore_Drm_Seat;

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
 * @li @ref Ecore_Drm_Tty_Group
 * 
 */

EAPI int ecore_drm_init(void);
EAPI int ecore_drm_shutdown(void);

EAPI Ecore_Drm_Device *ecore_drm_device_find(const char *name, const char *seat);
EAPI void ecore_drm_device_free(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_open(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_close(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_master_get(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_master_set(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_master_drop(Ecore_Drm_Device *dev);

EAPI Eina_Bool ecore_drm_tty_open(Ecore_Drm_Device *dev, const char *name);
EAPI Eina_Bool ecore_drm_tty_close(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_tty_release(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_tty_acquire(Ecore_Drm_Device *dev);

EAPI Eina_Bool ecore_drm_outputs_create(Ecore_Drm_Device *dev);
EAPI void ecore_drm_output_free(Ecore_Drm_Output *output);

EAPI Eina_Bool ecore_drm_inputs_create(Ecore_Drm_Device *dev);
EAPI void ecore_drm_inputs_destroy(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_inputs_enable(Ecore_Drm_Input *input);
//EAPI Eina_Bool ecore_drm_inputs_disable(Ecore_Drm_Input *input);

#endif
