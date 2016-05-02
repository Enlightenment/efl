#ifndef _ECORE_DRM2_H
# define _ECORE_DRM2_H

# include <Ecore.h>
# include <Elput.h>

# ifdef EAPI
#  undef EAPI
# endif

# ifdef _MSC_VER
#  ifdef BUILDING_DLL
#   define EAPI __declspec(dllexport)
#  else // ifdef BUILDING_DLL
#   define EAPI __declspec(dllimport)
#  endif // ifdef BUILDING_DLL
# else // ifdef _MSC_VER
#  ifdef __GNUC__
#   if __GNUC__ >= 4
#    define EAPI __attribute__ ((visibility("default")))
#   else // if __GNUC__ >= 4
#    define EAPI
#   endif // if __GNUC__ >= 4
#  else // ifdef __GNUC__
#   define EAPI
#  endif // ifdef __GNUC__
# endif // ifdef _MSC_VER

# ifdef EFL_BETA_API_SUPPORT

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm2_Device Ecore_Drm2_Device;

/**
 * @file
 * @brief Ecore functions for dealing with drm, virtual terminals
 *
 * @defgroup Ecore_Drm2_Group Ecore_Drm2 - Drm Integration
 * @ingroup Ecore
 *
 * Ecore_Drm2 provides a wrapper and functions for using libdrm
 *
 * @li @ref Ecore_Drm2_Init_Group
 * @li @ref Ecore_Drm2_Device_Group
 */

/**
 * @defgroup Ecore_Drm2_Init_Group Drm library Init and Shutdown functions
 *
 * Functions that start and shutdown the Ecore_Drm2 library
 */

/**
 * Initialize the Ecore_Drm2 library
 *
 * @return  The number of times the library has been initialized without
 *          being shut down. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm2_Init_Group
 * @since 1.18
 */
EAPI int ecore_drm2_init(void);

/**
 * Shutdown the Ecore_Drm2 library
 *
 * @return  The number of times the library has been initialized without
 *          being shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm2_Init_Group
 * @since 1.18
 */
EAPI int ecore_drm2_shutdown(void);

/**
 * @defgroup Ecore_Drm2_Device_Group Drm device functions
 *
 * Functions that deal with finding, opening, closing, or obtaining various
 * information about a drm device
 */

/**
 * Try to find a drm device on a given seat
 *
 * @param seat
 * @param tty
 * @param sync
 *
 * @return A newly allocated Ecore_Drm2_Device on success, NULL otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Device *ecore_drm2_device_find(const char *seat, unsigned int tty, Eina_Bool sync);

/**
 * Try to open a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @return A valid file descriptor if open succeeds, -1 otherwise.
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI int ecore_drm2_device_open(Ecore_Drm2_Device *device);

/**
 * Close an open Ecore_Drm2_Device
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_close(Ecore_Drm2_Device *device);

/**
 * Free a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_free(Ecore_Drm2_Device *device);

/**
 * Get the type of clock used by a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @return The clockid_t used by this drm device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI int ecore_drm2_device_clock_id_get(Ecore_Drm2_Device *device);

/**
 * Get the size of the cursor supported by a given Ecore_Drm2_Device
 *
 * @param device
 * @param width
 * @param height
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_cursor_size_get(Ecore_Drm2_Device *device, int *width, int *height);

# endif

#endif
