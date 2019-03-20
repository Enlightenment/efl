#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct
{
   const Efl_Class *inner_class;
   Eina_Stringshare *proxy_url;
   double dial_timeout;
   double timeout_inactivity;
   size_t max_queue_size_input;
   size_t max_queue_size_output;
   size_t read_chunk_size;
   Eina_Slice line_delimiter;
   struct {
      Eina_Bool proxy_url;
      Eina_Bool dial_timeout;
      Eina_Bool timeout_inactivity;
      Eina_Bool max_queue_size_input;
      Eina_Bool max_queue_size_output;
      Eina_Bool read_chunk_size;
      Eina_Bool line_delimiter;
   } pending;
} Efl_Net_Dialer_Simple_Data;

#define MY_CLASS EFL_NET_DIALER_SIMPLE_CLASS

EOLIAN static Efl_Object *
_efl_net_dialer_simple_efl_object_finalize(Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   if (efl_io_buffered_stream_inner_io_get(o)) goto end;

   if (!pd->inner_class)
     {
        ERR("no valid dialer was set with efl_io_buffered_stream_inner_io_set() and no class set with efl_net_dialer_simple_inner_class_set()!");
        return NULL;
     }
   else
     {
        Eo *dialer = efl_add(pd->inner_class, o);
        EINA_SAFETY_ON_NULL_RETURN_VAL(dialer, NULL);

        if (!efl_isa(dialer, EFL_NET_DIALER_INTERFACE))
          {
             ERR("class %s=%p doesn't implement Efl.Net.Dialer interface!", efl_class_name_get(pd->inner_class), pd->inner_class);
             efl_del(dialer);
             return NULL;
          }
        DBG("created new inner dialer %p (%s)", dialer, efl_class_name_get(efl_class_get(dialer)));

        efl_io_buffered_stream_inner_io_set(o, dialer);
     }

 end:
   return efl_finalize(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_dialer_simple_efl_object_invalidate(Eo *o, Efl_Net_Dialer_Simple_Data *pd EINA_UNUSED)
{
   Eo *inner_io;

   inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (inner_io)
     {
        efl_event_callback_forwarder_del(inner_io, EFL_NET_DIALER_EVENT_DIALER_ERROR, o);
        efl_event_callback_forwarder_del(inner_io, EFL_NET_DIALER_EVENT_DIALER_RESOLVED, o);
        efl_event_callback_forwarder_del(inner_io, EFL_NET_DIALER_EVENT_DIALER_CONNECTED, o);
        if (efl_parent_get(inner_io) == o)
          efl_parent_set(inner_io, NULL);
     }

   efl_invalidate(efl_super(o, EFL_NET_DIALER_SIMPLE_CLASS));
}

EOLIAN static void
_efl_net_dialer_simple_efl_object_destructor(Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   if (pd->inner_class) pd->inner_class = NULL;

   eina_stringshare_replace(&pd->proxy_url, NULL);
   if (pd->line_delimiter.mem)
     {
        free((void *)pd->line_delimiter.mem);
        pd->line_delimiter.mem = NULL;
     }

   efl_destructor(efl_super(o, EFL_NET_DIALER_SIMPLE_CLASS));
}

EOLIAN static void
_efl_net_dialer_simple_efl_io_buffered_stream_inner_io_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, Efl_Object *io)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(io, EFL_NET_DIALER_INTERFACE));
   efl_io_buffered_stream_inner_io_set(efl_super(o, MY_CLASS), io);
   efl_event_callback_forwarder_add(io, EFL_NET_DIALER_EVENT_DIALER_ERROR, o);
   efl_event_callback_forwarder_add(io, EFL_NET_DIALER_EVENT_DIALER_RESOLVED, o);
   efl_event_callback_forwarder_add(io, EFL_NET_DIALER_EVENT_DIALER_CONNECTED, o);
   /* apply pending dialer values */
   if (pd->pending.proxy_url)
     {
        pd->pending.proxy_url = EINA_FALSE;
        efl_net_dialer_proxy_set(io, pd->proxy_url);
        eina_stringshare_replace(&pd->proxy_url, NULL);
     }
   if (pd->pending.dial_timeout)
     {
        pd->pending.dial_timeout = EINA_FALSE;
        efl_net_dialer_timeout_dial_set(io, pd->dial_timeout);
     }

   /* apply pending io buffered stream (own) values */
   if (pd->pending.timeout_inactivity)
     {
        pd->pending.timeout_inactivity = EINA_FALSE;
        efl_io_buffered_stream_timeout_inactivity_set(o, pd->timeout_inactivity);
     }
   if (pd->pending.max_queue_size_input)
     {
        pd->pending.max_queue_size_input = EINA_FALSE;
        efl_io_buffered_stream_max_queue_size_input_set(o, pd->max_queue_size_input);
     }
   if (pd->pending.max_queue_size_output)
     {
        pd->pending.max_queue_size_output = EINA_FALSE;
        efl_io_buffered_stream_max_queue_size_output_set(o, pd->max_queue_size_output);
     }
   if (pd->pending.read_chunk_size)
     {
        pd->pending.read_chunk_size = EINA_FALSE;
        efl_io_buffered_stream_read_chunk_size_set(o, pd->read_chunk_size);
     }
   if (pd->pending.line_delimiter)
     {
        pd->pending.line_delimiter = EINA_FALSE;
        efl_io_buffered_stream_line_delimiter_set(o, pd->line_delimiter);
        free((void *)pd->line_delimiter.mem);
        pd->line_delimiter.mem = NULL;
     }
}

EOLIAN static Eina_Error
_efl_net_dialer_simple_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Simple_Data *pd EINA_UNUSED, const char *address)
{
   return efl_net_dialer_dial(efl_io_buffered_stream_inner_io_get(o), address);
}

EOLIAN static const char *
_efl_net_dialer_simple_efl_net_dialer_address_dial_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd EINA_UNUSED)
{
   return efl_net_dialer_address_dial_get(efl_io_buffered_stream_inner_io_get(o));
}

EOLIAN static Eina_Bool
_efl_net_dialer_simple_efl_net_dialer_connected_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd EINA_UNUSED)
{
   return efl_net_dialer_connected_get(efl_io_buffered_stream_inner_io_get(o));
}

EOLIAN static void
_efl_net_dialer_simple_efl_net_dialer_proxy_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, const char *proxy_url)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);

   if (!inner_io)
     {
        eina_stringshare_replace(&pd->proxy_url, proxy_url);
        pd->pending.proxy_url = EINA_TRUE;
        return;
     }
   efl_net_dialer_proxy_set(inner_io, proxy_url);
}

EOLIAN static const char *
_efl_net_dialer_simple_efl_net_dialer_proxy_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (!inner_io) return pd->proxy_url;
   return efl_net_dialer_proxy_get(inner_io);
}

EOLIAN static void
_efl_net_dialer_simple_efl_net_dialer_timeout_dial_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, double seconds)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);

   if (!inner_io)
     {
        pd->dial_timeout = seconds;
        pd->pending.dial_timeout = EINA_TRUE;
        return;
     }
   efl_net_dialer_timeout_dial_set(inner_io, seconds);
}

EOLIAN static double
_efl_net_dialer_simple_efl_net_dialer_timeout_dial_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (!inner_io) return pd->dial_timeout;
   return efl_net_dialer_timeout_dial_get(inner_io);
}

EOLIAN static void
_efl_net_dialer_simple_efl_io_buffered_stream_timeout_inactivity_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, double seconds)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);

   if (!inner_io)
     {
        pd->timeout_inactivity = seconds;
        pd->pending.timeout_inactivity = EINA_TRUE;
        return;
     }
   efl_io_buffered_stream_timeout_inactivity_set(efl_super(o, MY_CLASS), seconds);
}

EOLIAN static double
_efl_net_dialer_simple_efl_io_buffered_stream_timeout_inactivity_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (!inner_io) return pd->timeout_inactivity;
   return efl_io_buffered_stream_timeout_inactivity_get(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_dialer_simple_efl_io_buffered_stream_max_queue_size_input_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, size_t size)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);

   if (!inner_io)
     {
        pd->max_queue_size_input = size;
        pd->pending.max_queue_size_input = EINA_TRUE;
        return;
     }
   efl_io_buffered_stream_max_queue_size_input_set(efl_super(o, MY_CLASS), size);
}

EOLIAN static size_t
_efl_net_dialer_simple_efl_io_buffered_stream_max_queue_size_input_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (!inner_io) return pd->max_queue_size_input;
   return efl_io_buffered_stream_max_queue_size_input_get(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_dialer_simple_efl_io_buffered_stream_max_queue_size_output_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, size_t size)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);

   if (!inner_io)
     {
        pd->max_queue_size_output = size;
        pd->pending.max_queue_size_output = EINA_TRUE;
        return;
     }
   efl_io_buffered_stream_max_queue_size_output_set(efl_super(o, MY_CLASS), size);
}

EOLIAN static size_t
_efl_net_dialer_simple_efl_io_buffered_stream_max_queue_size_output_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (!inner_io) return pd->max_queue_size_output;
   return efl_io_buffered_stream_max_queue_size_output_get(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_dialer_simple_efl_io_buffered_stream_read_chunk_size_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, size_t size)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);

   if (!inner_io)
     {
        pd->read_chunk_size = size;
        pd->pending.read_chunk_size = EINA_TRUE;
        return;
     }
   efl_io_buffered_stream_read_chunk_size_set(efl_super(o, MY_CLASS), size);
}

EOLIAN static size_t
_efl_net_dialer_simple_efl_io_buffered_stream_read_chunk_size_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (!inner_io) return pd->read_chunk_size;
   return efl_io_buffered_stream_read_chunk_size_get(efl_super(o, MY_CLASS));
}


EOLIAN static void
_efl_net_dialer_simple_efl_io_buffered_stream_line_delimiter_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, Eina_Slice slice)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);

   if (!inner_io)
     {
        free((void *)pd->line_delimiter.mem);
        if (!slice.len)
          {
             pd->line_delimiter.mem = NULL;
             pd->line_delimiter.len = 0;
          }
        else
          {
             char *mem;
             pd->line_delimiter.mem = mem = malloc(slice.len + 1);
             EINA_SAFETY_ON_NULL_RETURN(pd->line_delimiter.mem);
             memcpy(mem, slice.mem, slice.len);
             mem[slice.len] = '\0';
             pd->line_delimiter.len = slice.len;
          }

        pd->pending.line_delimiter = EINA_TRUE;
        return;
     }
   efl_io_buffered_stream_line_delimiter_set(efl_super(o, MY_CLASS), slice);
}

EOLIAN static Eina_Slice
_efl_net_dialer_simple_efl_io_buffered_stream_line_delimiter_get(const Eo *o, Efl_Net_Dialer_Simple_Data *pd)
{
   Eo *inner_io = efl_io_buffered_stream_inner_io_get(o);
   if (!inner_io) return pd->line_delimiter;
   return efl_io_buffered_stream_line_delimiter_get(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_dialer_simple_inner_class_set(Eo *o, Efl_Net_Dialer_Simple_Data *pd, const Efl_Class *klass)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   EINA_SAFETY_ON_NULL_RETURN(klass);
   pd->inner_class = klass;
   DBG("%p inner_class=%p %s", o, klass, efl_class_name_get(klass));
}

EOLIAN static const Efl_Class *
_efl_net_dialer_simple_inner_class_get(const Eo *o EINA_UNUSED, Efl_Net_Dialer_Simple_Data *pd)
{
   return pd->inner_class;
}

#include "efl_net_dialer_simple.eo.c"
