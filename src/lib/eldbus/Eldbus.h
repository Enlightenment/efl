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

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ELDBUS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ELDBUS_BUILD */
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
#define ELDBUS_VERSION_MAJOR EFL_VERSION_MAJOR
#define ELDBUS_VERSION_MINOR EFL_VERSION_MINOR

#define ELDBUS_FDO_BUS "org.freedesktop.DBus"
#define ELDBUS_FDO_PATH "/org/freedesktop/DBus"
#define ELDBUS_FDO_INTERFACE ELDBUS_FDO_BUS
#define ELDBUS_FDO_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"
#define ELDBUS_FDO_INTERFACE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"
#define ELDBUS_FDO_INTERFACE_INTROSPECTABLE "org.freedesktop.DBus.Introspectable"
#define ELDBUS_FDO_INTEFACE_PEER "org.freedesktop.DBus.Peer"
#define ELDBUS_ERROR_PENDING_CANCELED "org.enlightenment.DBus.Canceled"
#define ELDBUS_ERROR_PENDING_TIMEOUT "org.freedesktop.DBus.Error.NoReply"

typedef struct _Eldbus_Version
{
   int major;
   int minor;
   int micro;
   int revision;
} Eldbus_Version;

EAPI extern const Eldbus_Version * eldbus_version;

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

typedef void                       (*Eldbus_Free_Cb)(void *data, const void *deadptr);

/**
 * @typedef Eldbus_Connection
 *
 * Represents a connection of one the type of connection with the DBus daemon.
 */
typedef struct _Eldbus_Connection     Eldbus_Connection;
/**
 * @typedef Eldbus_Object
 *
 * Represents an object path already attached with bus name or unique id.
 */
typedef struct _Eldbus_Object         Eldbus_Object;
/**
 * @typedef Eldbus_Proxy
 *
 * Represents an interface of an object path.
 */
typedef struct _Eldbus_Proxy          Eldbus_Proxy;
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

typedef void (*Eldbus_Message_Cb)(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending);
typedef void (*Eldbus_Signal_Cb)(void *data, const Eldbus_Message *msg);
/**
 * @}
 */

#include "eldbus_connection.h"
#include "eldbus_message.h"
#include "eldbus_signal_handler.h"
#include "eldbus_pending.h"
#include "eldbus_object.h"
#include "eldbus_proxy.h"
#include "eldbus_freedesktop.h"
#include "eldbus_service.h"

#ifdef __cplusplus
}
#endif

#endif
