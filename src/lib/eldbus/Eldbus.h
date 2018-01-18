/**
 * @page eldbus_main Eldbus
 *
 * @date 2012 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref eldbus_main_intro
 * @li @ref eldbus_main_compiling
 * @li @ref eldbus_main_next_steps
 * @li @ref eldbus_main_intro_example
 *
 * @section eldbus_main_intro Introduction
 *
 * Eldbus is a wrapper around the
 * <a href="http://www.freedesktop.org/wiki/Software/dbus">dbus</a>
 * library, which is a message bus system. It also implements a set of
 * specifications using dbus as interprocess communication.
 *
 * @subsection eldbus_modules_sec Modules
 *
 * <ul>
 * <li> @ref Eldbus_Core
 * <li> @ref Eldbus_Conneciton
 * <li> @ref Eldbus_Object_Mapper
 * <li> @ref Eldbus_Proxy
 * <li> @ref Eldbus_Message
 *      <ul>
 *              <li>@ref Eldbus_Message_Iter
 *              <li>@ref Eldbus_Message_Helpers
 *              <li>@ref Eina_Value
 *      </ul>
 * <li> @ref Eldbus_Signal_Handler
 * <li> @ref Eldbus_Pending
 * <li> @ref Eldbus_Service
 * <li> @ref Eldbus_Basic
 * </ul>
 *
 * @section eldbus_main_compiling How to compile
 *
 * Eldbus is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags eldbus`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs eldbus`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section eldbus_main_next_steps Next Steps
 *
 * After you understood what Eldbus is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 * @li @ref Eldbus_Core for library init, shutdown and getting a connection.
 * @li @ref Eldbus_Proxy to easily bind a client object to an interface.
 * @li @ref Eldbus_Object_Mapper to monitor server objects and properties.
 *
 * @section eldbus_main_intro_example Introductory Example
 *
 * @include ofono-dial.c
 *
 * More examples can be found at @ref eldbus_examples.
 */
#ifndef ELDBUS_H
#define ELDBUS_H

#include <Eina.h>
#include <stdarg.h>
#include <Efl_Config.h>
#include <Eo.h>
#include <Efl.h>

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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Eldbus_Core Core
 * @ingroup Eldbus
 *
 * @{
 */
#define ELDBUS_VERSION_MAJOR EFL_VERSION_MAJOR /**< Eldbus version major number */
#define ELDBUS_VERSION_MINOR EFL_VERSION_MINOR /**< Eldbus version minor number */

#define ELDBUS_FDO_BUS "org.freedesktop.DBus" /**< DBus message bus name */
#define ELDBUS_FDO_PATH "/org/freedesktop/DBus" /**< Object that implements message bus interface */
#define ELDBUS_FDO_INTERFACE ELDBUS_FDO_BUS /**< DBus message bus interface */
#define ELDBUS_FDO_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties" /**< DBus Properties interface */
#define ELDBUS_FDO_INTERFACE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager" /**< DBus ObjectManager interface */
#define ELDBUS_FDO_INTERFACE_INTROSPECTABLE "org.freedesktop.DBus.Introspectable" /**< DBus Introspectable interface */
#define ELDBUS_FDO_INTEFACE_PEER "org.freedesktop.DBus.Peer" /**< DBus Peer interface */
#define ELDBUS_ERROR_PENDING_CANCELED "org.enlightenment.DBus.Canceled" /**< Canceled error answer */
#define ELDBUS_ERROR_PENDING_TIMEOUT "org.freedesktop.DBus.Error.NoReply" /**< Timeout error answer */

/**
 * @typedef Eldbus_Version
 * Represents the current version of Eldbus
 */
typedef struct _Eldbus_Version
{
   int major; /**< major (binary or source incompatible changes) */
   int minor; /**< minor (new features, bugfixes, major improvements version) */
   int micro; /**< micro (bugfix, internal improvements, no new features version) */
   int revision; /**< git revision (0 if a proper release or the git revision number Eldbus is built from) */
} Eldbus_Version;

EAPI extern const Eldbus_Version * eldbus_version; /**< Global Eldbus_Version object */

/**
 * @brief Initialize eldbus.
 *
 * @return 1 or greater on success, 0 otherwise
 */
EAPI int eldbus_init(void);
/**
 * @brief Shutdown eldbus.
 *
 * @return 0 if e_dbus shuts down, greater than 0 otherwise.
 */
EAPI int eldbus_shutdown(void);

/**
 * @typedef Eldbus_Free_Cb
 *
 * Callback that is called when the connection is freed.
 */
typedef void                       (*Eldbus_Free_Cb)(void *data, const void *deadptr);
/**
 * @typedef Eldbus_Message
 *
 * Represents the way data is sent and received in DBus.
 */
typedef struct _Eldbus_Message        Eldbus_Message;
/**
 * @typedef Eldbus_Message_Iter
 *
 * Represents an iterator over a complex message type (array, dict, struct,
 * or variant). Its life is bound to the message that contains
 * it. The same applies to the returned data.
 */
typedef struct _Eldbus_Message_Iter Eldbus_Message_Iter;
/**
 * @typedef Eldbus_Pending
 *
 * Represents a message that has been sent but has not yet reached its
 * destination.
 */
typedef struct _Eldbus_Pending        Eldbus_Pending;

/**
 * @typedef Eldbus_Signal_Handler
 *
 * Represents a listener that will listen for signals emitted by other
 * applications.
 */
typedef struct _Eldbus_Signal_Handler Eldbus_Signal_Handler;

/**
 * @typedef Eldbus_Message_Cb
 *
 * Callback that is called when answer of a method call message comes.
 */
typedef void (*Eldbus_Message_Cb)(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending);

/**
 * @typedef Eldbus_Signal_Cb
 *
 * Callback that is called when a signal is received.
 */
typedef void (*Eldbus_Signal_Cb)(void *data, const Eldbus_Message *msg);
/**
 * @}
 */

#include "eldbus_types.eot.h"

#include "eldbus_connection.h"
#include "eldbus_message.h"
#include "eldbus_signal_handler.h"
#include "eldbus_pending.h"
#include "eldbus_object.h"
#include "eldbus_proxy.h"
#include "eldbus_freedesktop.h"
#include "eldbus_service.h"
#include "eldbus_introspection.h"

#ifdef EFL_BETA_API_SUPPORT

#include "eldbus_model_arguments.eo.h"
#include "eldbus_model_connection.eo.h"
#include "eldbus_model_method.eo.h"
#include "eldbus_model_object.eo.h"
#include "eldbus_model_proxy.eo.h"
#include "eldbus_model_signal.eo.h"

#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
