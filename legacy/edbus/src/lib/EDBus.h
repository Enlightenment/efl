#ifndef EDBUS_H
#define EDBUS_H

#include <Eina.h>
#include <stdarg.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EDBUS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EDBUS_BUILD */
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EDBus_Core Core
 *
 * @{
 */
#define EDBUS_VERSION_MAJOR 1
#define EDBUS_VERSION_MINOR 6

#define EDBUS_FDO_BUS "org.freedesktop.DBus"
#define EDBUS_FDO_PATH "/org/freedesktop/DBus"
#define EDBUS_FDO_INTERFACE EDBUS_FDO_BUS
#define EDBUS_FDO_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"

typedef struct _EDBus_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} EDBus_Version;

EAPI extern const EDBus_Version * edbus_version;

/**
 * @brief Initialize edbus.
 *
 * @return 1 or greater on success, 0 otherwise
 */
EAPI int edbus_init(void);
/**
 * @brief Shutdown edbus.
 *
 * @return 0 if e_dbus shuts down, greater than 0 otherwise.
 */
EAPI int edbus_shutdown(void);

typedef void                       (*EDBus_Free_Cb)(void *data, const void *deadptr);

/**
 * @typedef EDBus_Connection
 *
 * Represents a connection of one the type of connection with the DBus daemon.
 */
typedef struct _EDBus_Connection     EDBus_Connection;
/**
 * @typedef EDBus_Object
 *
 * Represents an object path already attached with bus name or unique id.
 */
typedef struct _EDBus_Object         EDBus_Object;
/**
 * @typedef EDBus_Proxy
 *
 * Represents an interface of an object path.
 */
typedef struct _EDBus_Proxy          EDBus_Proxy;
/**
 * @typedef EDBus_Message
 *
 * Represents the way data is sent and received in DBus.
 */
typedef struct _EDBus_Message        EDBus_Message;
/**
 * @typedef EDBus_Message_Iter
 *
 * Represents an iterator over a complex message type (array, dict, struct,
 * or variant). Its life is bound to the message that contains
 * it. The same applies to the returned data.
 */
typedef struct _EDBus_Message_Iter EDBus_Message_Iter;
/**
 * @typedef EDBus_Pending
 *
 * Represents a message that has been sent but has not yet reached its
 * destination.
 */
typedef struct _EDBus_Pending        EDBus_Pending;
/**
 * @typedef EDBus_Signal_Handler
 *
 * Represents a listener that will listen for signals emitted by other
 * applications.
 */
typedef struct _EDBus_Signal_Handler EDBus_Signal_Handler;

typedef void (*EDBus_Message_Cb)(void *data, const EDBus_Message *msg, EDBus_Pending *pending);
typedef void (*EDBus_Signal_Cb)(void *data, const EDBus_Message *msg);
/**
 * @}
 */

#include "edbus_connection.h"
#include "edbus_message.h"
#include "edbus_signal_handler.h"
#include "edbus_pending.h"
#include "edbus_object.h"
#include "edbus_proxy.h"
#include "edbus_freedesktop.h"
#include "edbus_service.h"
#include "edbus_message_helper.h"
#include "edbus_message_to_eina_value.h"
#include "edbus_message_from_eina_value.h"

#ifdef __cplusplus
}
#endif

#endif
