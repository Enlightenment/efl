#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct
{

} Efl_Net_Socket_Simple_Data;

#define MY_CLASS EFL_NET_SOCKET_SIMPLE_CLASS

EOLIAN static void
_efl_net_socket_simple_efl_io_buffered_stream_inner_io_set(Eo *o, Efl_Net_Socket_Simple_Data *pd EINA_UNUSED, Efl_Object *io)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(io, EFL_NET_SOCKET_INTERFACE));
   efl_io_buffered_stream_inner_io_set(efl_super(o, MY_CLASS), io);
}

EOLIAN static const char *
_efl_net_socket_simple_efl_net_socket_address_local_get(const Eo *o, Efl_Net_Socket_Simple_Data *pd EINA_UNUSED)
{
   return efl_net_socket_address_local_get(efl_io_buffered_stream_inner_io_get(o));
}

EOLIAN static const char *
_efl_net_socket_simple_efl_net_socket_address_remote_get(const Eo *o, Efl_Net_Socket_Simple_Data *pd EINA_UNUSED)
{
   return efl_net_socket_address_remote_get(efl_io_buffered_stream_inner_io_get(o));
}

#include "efl_net_socket_simple.eo.c"
