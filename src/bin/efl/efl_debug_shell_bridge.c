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

#include <Eo.h>
#include <Eina.h>
#include <Ecore.h>

#include <unistd.h>

static Eina_Debug_Session *_shell_session = NULL, *_local_session = NULL;

static int _nb_msgs = 0;

static Eina_Bool
_check_nb_msgs(void *data EINA_UNUSED)
{
   static int last_nb_msgs = 0;
   if (last_nb_msgs == _nb_msgs)
     {
        ecore_main_loop_quit();
        return EINA_FALSE;
     }
   last_nb_msgs = _nb_msgs;
   return EINA_TRUE;
}

static Eina_Debug_Error
_forward(Eina_Debug_Session *session, void *buffer)
{
   Eina_Debug_Packet_Header *hdr = buffer;
   char *payload = ((char *)buffer) + sizeof(*hdr);
   int size = hdr->size - sizeof(*hdr);
   eina_debug_session_send_to_thread(session == _local_session ? _shell_session : _local_session,
         hdr->cid, hdr->thread_id, hdr->opcode, payload, size);
   if (session == _shell_session) _nb_msgs = (_nb_msgs + 1) % 1000000;
   free(buffer);
   return EINA_DEBUG_OK;
}

int
main(int argc, char **argv)
{
   (void)argc;
   (void)argv;

   eina_debug_disable();
   eina_init();
   ecore_init();

   _local_session = eina_debug_local_connect(EINA_TRUE);
   eina_debug_session_dispatch_override(_local_session, _forward);

   _shell_session = eina_debug_fds_attach(STDIN_FILENO, STDOUT_FILENO);
   eina_debug_session_dispatch_override(_shell_session, _forward);
   eina_debug_session_shell_codec_enable(_shell_session);

   ecore_timer_add(30.0, _check_nb_msgs, NULL);
   ecore_main_loop_begin();

   ecore_shutdown();
   eina_shutdown();
}

