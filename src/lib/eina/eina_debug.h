/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_DEBUG_H_
# define EINA_DEBUG_H_

# include "eina_config.h"
# include "eina_list.h"

/**
 * @page eina_debug_main Eina Debug
 *
 * @date 2015 (created)
 */

/**
 * @addtogroup Eina_Debug
 * @{
 */

#define SERVER_PATH ".edebug"
#define SERVER_NAME "efl_debug"
#define SERVER_MASTER_PORT 0
#define SERVER_SLAVE_PORT 1

typedef enum
{
   EINA_DEBUG_OK,
   EINA_DEBUG_ERROR,
   EINA_DEBUG_AGAIN
} Eina_Debug_Error;

enum
{
   EINA_DEBUG_OPCODE_INVALID = -1, /**< Invalid opcode value */
   EINA_DEBUG_OPCODE_REGISTER = 0, /**< Opcode used to register other opcodes */
   EINA_DEBUG_OPCODE_HELLO = 1 /**< Opcode used to send greetings to the daemon */
};

/**
 * @typedef Eina_Debug_Session
 *
 * A handle used to interact with the debug daemon.
 * It contains all the information related to this connection and needed
 * to send/receive/dispatch/...
 */
typedef struct _Eina_Debug_Session Eina_Debug_Session;

/**
 * @typedef Eina_Debug_Cb
 *
 * A callback invoked when a specific packet is received.
 *
 * @param session the session
 * @param srcid the source id
 * @param buffer the packet payload data. It doesn't contain any transport information.
 * @param size the packet payload size
 */
typedef Eina_Debug_Error (*Eina_Debug_Cb)(Eina_Debug_Session *session, int srcid, void *buffer, int size);

/**
 * @typedef Eina_Debug_Opcode_Status_Cb
 *
 * When the opcodes ids are retrieved, this callback is invoked with a true
 * status.
 * When a disconnection to the daemon is happening, the opcodes ids are set
 * as invalid and this callback is invoked with a false status. The upper
 * layer should not try to send more requests until a new connection is
 * established.
 *
 * @param status EINA_TRUE if opcodes have been received from the daemon, EINA_FALSE otherwise.
 */
typedef void (*Eina_Debug_Opcode_Status_Cb)(Eina_Bool status);

/**
 * @typedef Eina_Debug_Dispatch_Cb
 *
 * Dispatcher callback prototype used to override the default dispatcher of a
 * session.
 *
 * @param session the session
 * @param buffer the packet received
 *
 * The given packet is the entire data received, including the header.
 */
typedef Eina_Debug_Error (*Eina_Debug_Dispatch_Cb)(Eina_Debug_Session *session, void *buffer);

/**
 * @typedef Eina_Debug_Timer_Cb
 *
 * A callback for a timer
 */
typedef Eina_Bool (*Eina_Debug_Timer_Cb)(void *);

/**
 * @typedef Eina_Debug_Packet_Header
 *
 * Header of Eina Debug packet
 */
typedef struct
{
   int size; /**< Packet size after this element */
   /**<
    * During sending, it corresponds to the id of the destination. During reception, it is the id of the source
    * The daemon is in charge of swapping the id before forwarding the packet to the destination.
    */
   int cid;
   int thread_id;
   int opcode; /**< Opcode of the packet */
} Eina_Debug_Packet_Header;

/**
 * @typedef Eina_Debug_Opcode
 *
 * Structure to describe information for an opcode. It is used to register new
 * opcodes.
 */
typedef struct
{
   char *opcode_name; /**< Opcode string. On registration, the daemon uses it to calculate an opcode id */
   int *opcode_id; /**< A pointer to store the opcode id received from the daemon */
   Eina_Debug_Cb cb; /**< Callback to call when a packet corresponding to the opcode is received */
} Eina_Debug_Opcode;

/**
 * @brief Disable debugging
 *
 * Useful for applications that don't want debugging. The debug daemon is one
 * of them.
 * Need to be invoked before eina_init. Otherwise it can't have any effect.
 */
EAPI void eina_debug_disable(void);

/**
 * @brief Connect to the local daemon
 *
 * @param is_master true if the application is a debugger. EINA_FALSE otherwise.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Debug_Session *eina_debug_local_connect(Eina_Bool is_master);

/**
 * @brief Terminate the session
 *
 * @param session the session to terminate
 *
 */
EAPI void eina_debug_session_terminate(Eina_Debug_Session *session);

/**
 * @brief Override the dispatcher of a specific session
 *
 * For example, it can be used to forward a packet to the main thread and to
 * use the default dispatcher there.
 * All the packets received in this session will use this dispatcher.
 *
 * @param session the session
 * @disp_cb the new dispatcher for the given session
 */
EAPI void eina_debug_session_dispatch_override(Eina_Debug_Session *session, Eina_Debug_Dispatch_Cb disp_cb);

/**
 * @brief Dispatch a given packet according to its header.
 *
 * This function checks the header contained into the packet and invokes
 * the correct callback according to the opcode.
 * This is the default dispatcher.
 *
 * @param session the session
 * @param buffer the packet
 *
 * @return EINA_DEBUG_OK on success, EINA_DEBUG_ERROR if the packet is not as expected.
 */
EAPI Eina_Debug_Error eina_debug_dispatch(Eina_Debug_Session *session, void *buffer);

/**
 * @brief Register opcodes to a session
 *
 * This function registers opcodes for the given session. If the session is not
 * connected, the request is not sent to the daemon. Otherwise, the request for
 * the opcodes ids is sent.
 * On the reception from the daemon, status_cb function is invoked to inform
 * the requester that the opcodes can now be used.
 */
EAPI void eina_debug_opcodes_register(Eina_Debug_Session *session,
      const Eina_Debug_Opcode ops[], Eina_Debug_Opcode_Status_Cb status_cb);

/**
 * @brief Send a packet to the given destination
 *
 * The packet will be treated by the debug thread itself.
 *
 * @param session the session to use to send the packet
 * @param dest_id the destination id to send the packet to
 * @param op the opcode for this packet
 * @param data payload to send
 * @param size payload size
 *
 * @return the number of sent bytes
 */
EAPI int eina_debug_session_send(Eina_Debug_Session *session, int dest_id, int op, void *data, int size);

/**
 * @brief Send a packet to the given thread of the given destination
 *
 * If the thread is 0x0, the packet will be treated by the debug thread itself.
 * If the thread is 0xFF..FF, the packet will be broadcasted to all the threads.
 * Otherwise, the packet will be treated by the specific thread.
 *
 * @param session the session to use to send the packet
 * @param dest_id the destination id to send the packet to
 * @param thread_id the thread to send the packet to.
 * @param op the opcode for this packet
 * @param data payload to send
 * @param size payload size
 *
 * @return the number of sent bytes
 */
EAPI int eina_debug_session_send_to_thread(Eina_Debug_Session *session, int dest_id, int thread_id, int op, void *data, int size);

/**
 * @brief Add a timer
 *
 * Needed for polling debug
 *
 * @param timeout_ms timeout in ms
 * @param cb callback to call when the timeout is reached
 * @param data user data
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 */
EAPI Eina_Bool eina_debug_timer_add(unsigned int timeout_ms, Eina_Debug_Timer_Cb cb, void *data);

EAPI int eina_debug_thread_id_get(void);
#endif
/**
 * @}
 */
