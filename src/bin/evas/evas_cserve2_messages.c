#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <string.h>

#include "evas_cserve2.h"

// #define DEBUG_MSG 1

static void
debug_msg(const char *typestr, const void *buf, int size)
{
#ifdef DEBUG_MSG
    const char *str = buf;
    int i;

    printf("message %s: ", typestr);
    for (i = 0; i < size; i++)
      printf("%x ", str[i]);

    printf("\n");
#else
    (void)typestr;
    (void)buf;
    (void)size;
#endif
}

static void
_client_msg_allocate_buf(Client *client, int msgsize)
{
   client->msg.reading = EINA_TRUE;
   client->msg.buf = malloc(msgsize + 1);
   client->msg.size = msgsize;
   client->msg.done = 0;
}

static void
_client_msg_free(Client *client)
{
   client->msg.reading = EINA_FALSE;
   free(client->msg.buf);
   client->msg.buf = NULL;
}

static void
_client_msg_parse(Client *client)
{
   Msg_Base *msg = (Msg_Base *)client->msg.buf;
   DBG("Message received. Size: %d; type = %d",
       client->msg.size, msg->type);

   cserve2_command_run(client, msg->type);
}

static void
_client_msg_read(Client *client, int done)
{
   client->msg.done += done;
   if (client->msg.done == client->msg.size)
     {
        debug_msg("received", client->msg.buf, client->msg.size);
        _client_msg_parse(client);
        _client_msg_free(client);
     }
}

void
cserve2_message_handler(int fd EINA_UNUSED, Fd_Flags flags, void *data)
{
   Client *client = data;
   int len;
   int msgsize;

   if (flags & FD_ERROR)
     {
        ERR("Error on socket for client: %d", client->id);
        goto client_close;
     }

   if (flags & FD_WRITE)
     cserve2_client_deliver(client);

   if (!(flags & FD_READ))
     return;

   if (!client->msg.reading)
     len = cserve2_client_read(client, &msgsize, sizeof(msgsize));
   else
     len = cserve2_client_read(client, &client->msg.buf[client->msg.done],
                               client->msg.size - client->msg.done);

   if (!len)
     {
        INF("Client %d connection closed.", client->id);
        goto client_close;
     }

   if (len < 0)
     {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
          {
             WRN("No data to read but the message handler was called.");
             return;
          }
        WRN("Error when reading message from client: \"%s\"",
            strerror(errno));
        // FIXME: Should we close the connection, or just send an ERROR
        // message?
        goto client_close;
     }

   if (!client->msg.reading)
     _client_msg_allocate_buf(client, msgsize);
   else
     _client_msg_read(client, len);

   return;

client_close:
   if (client->msg.reading)
     _client_msg_free(client);
   cserve2_client_del(client);
}

void
cserve2_client_deliver(Client *client)
{
   size_t sent, size;
   const char *str;

   if (!client->msg.pending)
     {
        Fd_Flags cur_flags;
        cserve2_fd_watch_flags_get(client->socket, &cur_flags);
        cur_flags ^= FD_WRITE;
        cserve2_fd_watch_flags_set(client->socket, cur_flags);
        return;
     }

   size = eina_binbuf_length_get(client->msg.pending);
   str = (const char *)eina_binbuf_string_get(client->msg.pending);
   sent = cserve2_client_write(client, str, size);
   if (sent == size)
     {
        eina_binbuf_free(client->msg.pending);
        client->msg.pending = NULL;
        return;
     }

   eina_binbuf_remove(client->msg.pending, 0, sent);
}

ssize_t
cserve2_client_send(Client *client, const void *data, size_t size)
{
   ssize_t sent;

   debug_msg("sent", data, size);
   if (client->msg.pending)
     {
        eina_binbuf_append_length
           (client->msg.pending, (unsigned char *)data, size);
        return size;
     }

   sent = cserve2_client_write(client, data, size);
   if ((sent < 0) && ((errno != EAGAIN) && (errno != EWOULDBLOCK)))
     {
        // FIXME: Big error when writing on the socket to the client,
        // so we must close the connection to the client and remove
        // its references inside our cache.
        WRN("Error on socket with client %d: %s", client->id, strerror(errno));
        return sent;
     }
   if (sent < 0)
     sent = 0;
   if (sent < (int)size)
     {
        Fd_Flags cur_flags;
        client->msg.pending = eina_binbuf_new();
        eina_binbuf_append_length
           (client->msg.pending, (unsigned char *)data + sent, size - sent);
        cserve2_fd_watch_flags_get(client->socket, &cur_flags);
        cur_flags |= FD_WRITE;
        cserve2_fd_watch_flags_set(client->socket, cur_flags);
     }
   return size;
}
