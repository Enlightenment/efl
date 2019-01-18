#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1

#include <Ecore.h>
#include "ecore_private.h"

typedef struct
{
   Eo *inner_io;
   Eo *incoming;
   Eo *outgoing;
   Eo *sender;
   Eo *receiver;
   Eina_Bool is_closing;
   Eina_Bool closed;
   Eina_Bool eos;
   Eina_Bool can_read;
   Eina_Bool can_write;
   Eina_Bool is_closer;
   Eina_Bool is_finished;
} Efl_Io_Buffered_Stream_Data;

#define MY_CLASS EFL_IO_BUFFERED_STREAM_CLASS

static void
_efl_io_buffered_stream_error(void *data, const Efl_Event *event)
{
   Eo *o = data;
   Eina_Error *perr = event->info;
   DBG("%p %s error: %s", o, efl_name_get(event->object), eina_error_msg_get(*perr));
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_ERROR, event->info);
}

static void
_efl_io_buffered_stream_copier_progress(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_PROGRESS, NULL);
}

static void
_efl_io_buffered_stream_incoming_can_read_changed(void *data, const Efl_Event *event)
{
   Eo *o = data;
   if (efl_io_closer_closed_get(o)) return; /* already closed (or closing) */
   efl_io_reader_can_read_set(o, efl_io_reader_can_read_get(event->object));
}

static void
_efl_io_buffered_stream_incoming_slice_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_SLICE_CHANGED, NULL);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_io_buffered_stream_incoming_cbs,
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _efl_io_buffered_stream_incoming_can_read_changed },
                           { EFL_IO_QUEUE_EVENT_SLICE_CHANGED, _efl_io_buffered_stream_incoming_slice_changed });

static void
_efl_io_buffered_stream_receiver_line(void *data, const Efl_Event *event)
{
   Eo *o = data;
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_LINE, event->info);
}

static void
_efl_io_buffered_stream_receiver_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   if (efl_io_closer_closed_get(o)) return; /* already closed (or closing) */
   efl_io_reader_eos_set(o, EINA_TRUE);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_io_buffered_stream_receiver_cbs,
                           { EFL_IO_COPIER_EVENT_PROGRESS, _efl_io_buffered_stream_copier_progress },
                           { EFL_IO_COPIER_EVENT_DONE, _efl_io_buffered_stream_receiver_done },
                           { EFL_IO_COPIER_EVENT_LINE, _efl_io_buffered_stream_receiver_line },
                           { EFL_IO_COPIER_EVENT_ERROR, _efl_io_buffered_stream_error });


static void
_efl_io_buffered_stream_outgoing_can_write_changed(void *data, const Efl_Event *event)
{
   Eo *o = data;
   if (efl_io_closer_closed_get(o)) return; /* already closed (or closing) */
   efl_io_writer_can_write_set(o, efl_io_writer_can_write_get(event->object));
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_io_buffered_stream_outgoing_cbs,
                           { EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _efl_io_buffered_stream_outgoing_can_write_changed });

static void
_efl_io_buffered_stream_sender_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Io_Buffered_Stream_Data *pd = efl_data_scope_get(o, MY_CLASS);
   size_t pending = pd->receiver ? efl_io_copier_pending_size_get(pd->receiver) : 0;

   efl_ref(o);
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_PROGRESS, NULL);
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_WRITE_FINISHED, NULL);
   if ((!pd->receiver) || efl_io_copier_done_get(pd->receiver))
     {
        if (!pd->is_finished)
          {
             pd->is_finished = EINA_TRUE;
             efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, NULL);
          }
     }
   else
     DBG("%p sender done, waiting for receiver to process %zd to call it 'finished'", o, pending);
   efl_unref(o);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_io_buffered_stream_sender_cbs,
                           { EFL_IO_COPIER_EVENT_PROGRESS, _efl_io_buffered_stream_copier_progress },
                           { EFL_IO_COPIER_EVENT_DONE, _efl_io_buffered_stream_sender_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _efl_io_buffered_stream_error });

static void
_efl_io_buffered_stream_inner_io_del(void *data, const Efl_Event *event)
{
   Eo *o = data;
   Efl_Io_Buffered_Stream_Data *pd = efl_data_scope_get(o, MY_CLASS);
   DBG("%p the inner I/O %p was deleted", o, event->object);
   if (pd->inner_io == event->object)
     pd->inner_io = NULL;
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_io_buffered_stream_inner_io_cbs,
                           { EFL_EVENT_DEL, _efl_io_buffered_stream_inner_io_del });


EOLIAN static Efl_Object *
_efl_io_buffered_stream_efl_object_finalize(Eo *o, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->inner_io)
     {
        ERR("no valid I/O was set with efl_io_buffered_stream_inner_io_set()!");
        return NULL;
     }

   return efl_finalize(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_io_buffered_stream_efl_object_invalidate(Eo *o, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!efl_io_closer_closed_get(o))
     efl_io_closer_close(o);

   if (pd->inner_io)
     {
        efl_event_callback_array_del(pd->inner_io, _efl_io_buffered_stream_inner_io_cbs(), o);
        if (efl_parent_get(pd->inner_io) == o)
          efl_parent_set(pd->inner_io, NULL);
        else
          efl_unref(pd->inner_io); /* do not del, just take our ref */
        pd->inner_io = NULL;
     }

   pd->incoming = NULL;
   pd->outgoing = NULL;
   pd->sender = NULL;
   pd->receiver = NULL;

   if (!pd->is_finished)
     {
        pd->is_finished = EINA_TRUE;
        efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, NULL);
     }

   efl_invalidate(efl_super(o, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_io_buffered_stream_efl_io_closer_close(Eo *o, Efl_Io_Buffered_Stream_Data *pd)
{
   Eina_Error err = 0;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->closed, EINVAL);
   if (pd->is_closing) return 0;
   pd->is_closing = EINA_TRUE;

   if (pd->outgoing)
     {
        efl_io_queue_eos_mark(pd->outgoing);
        efl_io_copier_flush(pd->sender, EINA_FALSE, EINA_TRUE);
     }

   /* line delimiters may be holding a last chunk of data */
   if (pd->receiver) efl_io_copier_flush(pd->receiver, EINA_FALSE, EINA_TRUE);

   efl_io_writer_can_write_set(o, EINA_FALSE);
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);

   pd->closed = EINA_TRUE;
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);

   if (pd->sender && (!efl_io_closer_closed_get(pd->sender)))
     efl_io_closer_close(pd->sender);

   if (pd->receiver && (!efl_io_closer_closed_get(pd->receiver)))
     efl_io_closer_close(pd->receiver);

   return err;
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_efl_io_closer_closed_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   return pd->closed || efl_io_closer_closed_get(pd->inner_io);
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_efl_io_closer_close_on_exec_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   return efl_io_closer_close_on_exec_get(pd->inner_io);
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_efl_io_closer_close_on_exec_set(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, Eina_Bool value)
{
   return efl_io_closer_close_on_exec_set(pd->inner_io, value);
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_efl_io_closer_close_on_invalidate_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   return efl_io_closer_close_on_invalidate_get(pd->inner_io);
}

EOLIAN static void
_efl_io_buffered_stream_efl_io_closer_close_on_invalidate_set(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, Eina_Bool value)
{
   efl_io_closer_close_on_invalidate_set(pd->inner_io, value);
}

EOLIAN static Eina_Error
_efl_io_buffered_stream_efl_io_reader_read(Eo *o, Efl_Io_Buffered_Stream_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Eina_Error err;

   if (!pd->incoming)
     {
        WRN("%p reading from inner_io %p (%s) that doesn't implement Efl.Io.Reader",
            o, pd->inner_io, efl_class_name_get(efl_class_get(pd->inner_io)));
        return EINVAL;
     }

   err = efl_io_reader_read(pd->incoming, rw_slice);
   if (err && (err != EAGAIN))
     efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_ERROR, &err);
   return err;
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_efl_io_reader_can_read_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   return pd->can_read;
}

EOLIAN static void
_efl_io_buffered_stream_efl_io_reader_can_read_set(Eo *o, Efl_Io_Buffered_Stream_Data *pd EINA_UNUSED, Eina_Bool can_read)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_efl_io_reader_eos_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   return pd->eos;
}

EOLIAN static void
_efl_io_buffered_stream_efl_io_reader_eos_set(Eo *o, Efl_Io_Buffered_Stream_Data *pd, Eina_Bool is_eos)
{
   size_t pending = pd->sender ? efl_io_copier_pending_size_get(pd->sender) : 0;

   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->eos == is_eos) return;
   pd->eos = is_eos;
   if (!is_eos) return;

   efl_ref(o);
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_PROGRESS, NULL);
   efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
   efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_READ_FINISHED, NULL);
   if ((!pd->sender) || efl_io_copier_done_get(pd->sender))
     {
        if (!pd->is_finished)
          {
             pd->is_finished = EINA_TRUE;
             efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, NULL);
          }
     }
   else
     DBG("%p eos, waiting for sender process %zd to call 'finished'", o, pending);
   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_io_buffered_stream_efl_io_writer_write(Eo *o, Efl_Io_Buffered_Stream_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   Eina_Error err;

   if (!pd->outgoing)
     {
        WRN("%p writing to inner_io %p (%s) that doesn't implement Efl.Io.Writer",
            o, pd->inner_io, efl_class_name_get(efl_class_get(pd->inner_io)));
        return EINVAL;
     }

   err = efl_io_writer_write(pd->outgoing, slice, remaining);
   if (err && (err != EAGAIN))
     efl_event_callback_call(o, EFL_IO_BUFFERED_STREAM_EVENT_ERROR, &err);
   return err;
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_efl_io_writer_can_write_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_io_buffered_stream_efl_io_writer_can_write_set(Eo *o, Efl_Io_Buffered_Stream_Data *pd EINA_UNUSED, Eina_Bool can_write)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_closer_closed_get(o));
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

EOLIAN static void
_efl_io_buffered_stream_inner_io_set(Eo *o, Efl_Io_Buffered_Stream_Data *pd, Efl_Object *io)
{
   Eina_Bool is_reader, is_writer;

   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   EINA_SAFETY_ON_NULL_RETURN(io);
   EINA_SAFETY_ON_TRUE_RETURN(pd->inner_io != NULL);

   pd->is_closer = efl_isa(io, EFL_IO_CLOSER_INTERFACE);
   is_reader = efl_isa(io, EFL_IO_READER_INTERFACE);
   is_writer = efl_isa(io, EFL_IO_WRITER_INTERFACE);

   EINA_SAFETY_ON_TRUE_RETURN((!is_reader) && (!is_writer));

   pd->inner_io = efl_ref(io);
   efl_event_callback_array_add(io, _efl_io_buffered_stream_inner_io_cbs(), o);

   /* inner_io -> incoming */
   if (is_reader)
     {
        DBG("%p inner_io=%p (%s) is Efl.Io.Reader", o, io, efl_class_name_get(efl_class_get(io)));
        pd->incoming = efl_add(EFL_IO_QUEUE_CLASS, o,
                               efl_name_set(efl_added, "incoming"),
                               efl_event_callback_array_add(efl_added, _efl_io_buffered_stream_incoming_cbs(), o));
        EINA_SAFETY_ON_NULL_RETURN(pd->incoming);

        pd->receiver = efl_add(EFL_IO_COPIER_CLASS, o,
                               efl_name_set(efl_added, "receiver"),
                               efl_io_copier_buffer_limit_set(efl_added, 4096),
                               efl_io_copier_source_set(efl_added, io),
                               efl_io_copier_destination_set(efl_added, pd->incoming),
                               efl_io_closer_close_on_invalidate_set(efl_added, efl_io_closer_close_on_invalidate_get(io)),
                               efl_event_callback_array_add(efl_added, _efl_io_buffered_stream_receiver_cbs(), o));
        EINA_SAFETY_ON_NULL_RETURN(pd->receiver);
     }
   else
     {
        DBG("%p inner_io=%p (%s) is not Efl.Io.Reader", o, io, efl_class_name_get(efl_class_get(io)));
        efl_io_reader_eos_set(o, EINA_TRUE);
     }


   /* outgoing -> inner_io */
   if (is_writer)
     {
        DBG("%p inner_io=%p (%s) is Efl.Io.Writer", o, io, efl_class_name_get(efl_class_get(io)));
        pd->outgoing = efl_add(EFL_IO_QUEUE_CLASS, o,
                               efl_name_set(efl_added, "outgoing"),
                               efl_event_callback_array_add(efl_added, _efl_io_buffered_stream_outgoing_cbs(), o));
        EINA_SAFETY_ON_NULL_RETURN(pd->outgoing);

        pd->sender = efl_add(EFL_IO_COPIER_CLASS, o,
                             efl_name_set(efl_added, "sender"),
                             efl_io_copier_buffer_limit_set(efl_added, 4096),
                             efl_io_copier_source_set(efl_added, pd->outgoing),
                             efl_io_copier_destination_set(efl_added, io),
                             efl_io_closer_close_on_invalidate_set(efl_added, efl_io_closer_close_on_invalidate_get(io)),
                             efl_event_callback_array_add(efl_added, _efl_io_buffered_stream_sender_cbs(), o));
        EINA_SAFETY_ON_NULL_RETURN(pd->sender);
     }
   else
     DBG("%p inner_io=%p (%s) is not Efl.Io.Writer", o, io, efl_class_name_get(efl_class_get(io)));
}

EOLIAN static Efl_Object *
_efl_io_buffered_stream_inner_io_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   return pd->inner_io;
}

EOLIAN static void
_efl_io_buffered_stream_max_queue_size_input_set(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, size_t max_queue_size_input)
{
   if (!pd->incoming)
     {
        DBG("%p inner_io=%p (%s) is not Efl.Io.Reader, limit=%zu ignored", o, pd->inner_io, efl_class_name_get(efl_class_get(pd->inner_io)), max_queue_size_input);
        return;
     }
   efl_io_queue_limit_set(pd->incoming, max_queue_size_input);
}

EOLIAN static size_t
_efl_io_buffered_stream_max_queue_size_input_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->incoming) return 0;
   return efl_io_queue_limit_get(pd->incoming);
}

EOLIAN static void
_efl_io_buffered_stream_max_queue_size_output_set(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, size_t max_queue_size_output)
{
   if (!pd->outgoing)
     {
        DBG("%p inner_io=%p (%s) is not Efl.Io.Writer, limit=%zu ignored", o, pd->inner_io, efl_class_name_get(efl_class_get(pd->inner_io)), max_queue_size_output);
        return;
     }
   efl_io_queue_limit_set(pd->outgoing, max_queue_size_output);
}

EOLIAN static size_t
_efl_io_buffered_stream_max_queue_size_output_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->outgoing) return 0;
   return efl_io_queue_limit_get(pd->outgoing);
}

EOLIAN static void
_efl_io_buffered_stream_line_delimiter_set(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, Eina_Slice slice)
{
   if (!pd->receiver)
     {
        DBG("%p inner_io=%p (%s) is not Efl.Io.Reader, slice=" EINA_SLICE_FMT " ignored", o, pd->inner_io, efl_class_name_get(efl_class_get(pd->inner_io)), EINA_SLICE_PRINT(slice));
        return;
     }
   efl_io_copier_line_delimiter_set(pd->receiver, slice);
}

EOLIAN static Eina_Slice
_efl_io_buffered_stream_line_delimiter_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->receiver) return (Eina_Slice){};
   return efl_io_copier_line_delimiter_get(pd->receiver);
}

EOLIAN static void
_efl_io_buffered_stream_timeout_inactivity_set(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, double seconds)
{
   if (pd->receiver)
     efl_io_copier_timeout_inactivity_set(pd->receiver, seconds);
   if (pd->sender)
     efl_io_copier_timeout_inactivity_set(pd->sender, seconds);
}

EOLIAN static double
_efl_io_buffered_stream_timeout_inactivity_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (pd->receiver)
     return efl_io_copier_timeout_inactivity_get(pd->receiver);
   if (pd->sender)
     return efl_io_copier_timeout_inactivity_get(pd->sender);
   return 0.0;
}

EOLIAN static void
_efl_io_buffered_stream_read_chunk_size_set(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, size_t size)
{
   if (pd->sender)
     {
        efl_io_copier_buffer_limit_set(pd->sender, size);
        efl_io_copier_read_chunk_size_set(pd->sender, size);
     }

   if (!pd->receiver)
     {
        efl_io_copier_buffer_limit_set(pd->receiver, size);
        efl_io_copier_read_chunk_size_set(pd->receiver, size);
     }
}

EOLIAN static size_t
_efl_io_buffered_stream_read_chunk_size_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->receiver) return 0;
   return efl_io_copier_read_chunk_size_get(pd->receiver);
}

EOLIAN static size_t
_efl_io_buffered_stream_pending_write_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->outgoing) return 0;
   return efl_io_queue_usage_get(pd->outgoing);
}

EOLIAN static size_t
_efl_io_buffered_stream_pending_read_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->incoming) return 0;
   return efl_io_queue_usage_get(pd->incoming);
}

EOLIAN static void
_efl_io_buffered_stream_progress_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, size_t *pr, size_t *pw)
{
   uint64_t r = 0, w = 0;

   if (pd->sender) efl_io_copier_progress_get(pd->sender, NULL, &w, NULL);
   if (pd->receiver) efl_io_copier_progress_get(pd->receiver, &r, NULL, NULL);

   if (pr) *pr = r;
   if (pw) *pw = w;
}

EOLIAN static Eina_Slice
_efl_io_buffered_stream_slice_get(const Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   Eina_Slice slice = { };

   if (pd->incoming)
     slice = efl_io_queue_slice_get(pd->incoming);

   return slice;
}

EOLIAN static void
_efl_io_buffered_stream_discard(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd, size_t amount)
{
   if (!pd->incoming) return;
   efl_io_queue_discard(pd->incoming, amount);
}

EOLIAN static void
_efl_io_buffered_stream_clear(Eo *o EINA_UNUSED, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->incoming) return;
   efl_io_queue_clear(pd->incoming);
}

EOLIAN static void
_efl_io_buffered_stream_eos_mark(Eo *o, Efl_Io_Buffered_Stream_Data *pd)
{
   if (!pd->outgoing) return;
   DBG("%p mark eos", o);
   efl_io_queue_eos_mark(pd->outgoing);
}

EOLIAN static Eina_Bool
_efl_io_buffered_stream_flush(Eo *o, Efl_Io_Buffered_Stream_Data *pd, Eina_Bool may_block, Eina_Bool ignore_line_delimiter)
{
   size_t pending;
   Eina_Bool ret;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EINA_FALSE);

   if (!pd->outgoing) return EINA_TRUE;

   pending = efl_io_queue_usage_get(pd->outgoing);
   if (!pending)
     return EINA_TRUE;

   if (pd->is_closer && efl_io_closer_closed_get(pd->inner_io))
     {
        DBG("%p the inner I/O %p is already closed", o, pd->inner_io);
        return EINA_TRUE;
     }

   DBG("%p attempt to flush %zu bytes, may_block=%hhu, ignore_line_delimiter=%hhu...", o, pending, may_block, ignore_line_delimiter);
   ret = efl_io_copier_flush(pd->sender, may_block, ignore_line_delimiter);
   DBG("%p flushed, ret=%hhu, still pending=%zu", o, ret, efl_io_queue_usage_get(pd->outgoing));

   return ret;
}

#include "efl_io_buffered_stream.eo.c"
