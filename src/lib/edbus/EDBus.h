/**
 * @page edbus_main EDBus
 *
 * @date 2012 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref edbus_main_intro
 * @li @ref edbus_main_compiling
 * @li @ref edbus_main_next_steps
 * @li @ref edbus_main_intro_example
 *
 * @section edbus_main_intro Introduction
 *
 * EDBus is a wrapper around the
 * <a href="http://www.freedesktop.org/wiki/Software/dbus">dbus</a>
 * library, which is a message bus system. It also implements a set of
 * specifications using dbus as interprocess communication.
 *
 * @subsection edbus_modules_sec Modules
 *
 * <ul>
 * <li> @ref EDBus_Core
 * <li> @ref EDBus_Conneciton
 * <li> @ref EDBus_Object_Mapper
 * <li> @ref EDBus_Proxy
 * <li> @ref EDBus_Message
 *      <ul>
 *              <li>@ref EDBus_Message_Iter
 *              <li>@ref EDBus_Message_Helpers
 *              <li>@ref Eina_Value
 *      </ul>
 * <li> @ref EDBus_Signal_Handler
 * <li> @ref EDBus_Pending
 * <li> @ref EDBus_Service
 * <li> @ref EDBus_Basic
 * </ul>
 *
 * @section edbus_main_compiling How to compile
 *
 * EDBus is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags edbus2`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs edbus2`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section edbus_main_next_steps Next Steps
 *
 * After you understood what EDBus is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 * @li @ref EDBus_Core for library init, shutdown and getting a connection.
 * @li @ref EDBus_Proxy to easily bind a client object to an interface.
 * @li @ref EDBus_Object_Mapper to monitor server objects and properties.
 *
 * @section edbus_main_intro_example Introductory Example
 *
 * @include ofono-dial.c
 *
 * More examples can be found at @ref edbus_examples.
 */
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
 * @ingroup EDBus
 *
 * @{
 */
#define EDBUS_VERSION_MAJOR 1
#define EDBUS_VERSION_MINOR 6

#define EDBUS_FDO_BUS "org.freedesktop.DBus"
#define EDBUS_FDO_PATH "/org/freedesktop/DBus"
#define EDBUS_FDO_INTERFACE EDBUS_FDO_BUS
#define EDBUS_FDO_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"
#define EDBUS_FDO_INTERFACE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"
#define EDBUS_FDO_INTERFACE_INTROSPECTABLE "org.freedesktop.DBus.Introspectable"
#define EDBUS_FDO_INTEFACE_PEER "org.freedesktop.DBus.Peer"

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

#ifdef __cplusplus
}
#endif

#endif
