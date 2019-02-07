#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>

#include <Eina.h>

#include "Eet.h"
#include "Eet_private.h"

/* max message size: 1Gb - raised from original 64Kb */
#define MAX_MSG_SIZE (1024 * 1024 * 1024)
#define MAGIC_EET_DATA_PACKET 0x4270ACE1

struct _Eet_Connection
{
   Eet_Read_Cb  *eet_read_cb;
   Eet_Write_Cb *eet_write_cb;
   void         *user_data;

   size_t        allocated;
   size_t        size;
   size_t        received;

   void         *buffer;
};

EAPI Eet_Connection *
eet_connection_new(Eet_Read_Cb  *eet_read_cb,
                   Eet_Write_Cb *eet_write_cb,
                   const void   *user_data)
{
   Eet_Connection *conn;
   
   if ((!eet_read_cb) || (!eet_write_cb)) return NULL;
   
   conn = calloc(1, sizeof (Eet_Connection));
   if (!conn) return NULL;
   conn->eet_read_cb = eet_read_cb;
   conn->eet_write_cb = eet_write_cb;
   conn->user_data = (void *)user_data;
   return conn;
}

EAPI int
eet_connection_received(Eet_Connection *conn,
                        const void     *data,
                        size_t          size)
{
   if ((!conn) || (!data) || (!size)) return size;
   do
     {
        size_t copy_size;
        
        if (conn->size == 0)
          {
             const int *msg;
             size_t packet_size;
             
             if (size < (sizeof(int) * 2)) break;

             msg = data;
             /* Check the magic */
             if (eina_ntohl(msg[0]) != MAGIC_EET_DATA_PACKET) break;

             packet_size = eina_ntohl(msg[1]);
             /* Message should always be under MAX_MSG_SIZE */
             if (packet_size > MAX_MSG_SIZE) break;

             data = (void *)(msg + 2);
             size -= sizeof(int) * 2;
             if ((size_t)packet_size <= size)
               {
                  /* Not a partial receive, go the quick way. */
                  if (!conn->eet_read_cb(data, packet_size, conn->user_data))
                    break;
                  
                  data = (void *)((char *)data + packet_size);
                  size -= packet_size;
                  conn->received = 0;
                  continue;
               }
             conn->size = packet_size;
             if (conn->allocated < conn->size)
               {
                  void *tmp;
                  
                  tmp = realloc(conn->buffer, conn->size);
                  if (!tmp) break;
                  conn->buffer = tmp;
                  conn->allocated = conn->size;
               }
          }

        /* Partial receive */
        copy_size =
          (conn->size - conn->received >=
              size) ? size : conn->size - conn->received;
        memcpy((char *)conn->buffer + conn->received, data, copy_size);
        
        conn->received += copy_size;
        data = (void *)((char *)data + copy_size);
        size -= copy_size;
        
        if (conn->received == conn->size)
          {
             size_t data_size;
             
             data_size = conn->size;
             conn->size = 0;
             conn->received = 0;
             /* Completed a packet. */
             if (!conn->eet_read_cb(conn->buffer, data_size, conn->user_data))
               {
                  /* Something goes wrong. Stop now. */
                  size += data_size;
                  break;
               }
          }
     }
   while (size > 0);
   
   return size;
}

static Eina_Bool
_eet_connection_raw_send(Eet_Connection *conn,
                         void           *data,
                         int             data_size)
{
   int *message;

   /* Message should always be under MAX_MSG_SIZE */
   if (data_size > MAX_MSG_SIZE) return EINA_FALSE;
   message = malloc(data_size + (sizeof(int) * 2));
   message[0] = eina_htonl(MAGIC_EET_DATA_PACKET);
   message[1] = eina_htonl(data_size);
   memcpy(message + 2, data, data_size);
   conn->eet_write_cb(message,
                      data_size + (sizeof(int) * 2),
                      conn->user_data);

   free(message);
   return EINA_TRUE;
}

EAPI Eina_Bool
eet_connection_empty(Eet_Connection *conn)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, EINA_TRUE);
   return conn->size ? EINA_FALSE : EINA_TRUE;
}

EAPI Eina_Bool
eet_connection_send(Eet_Connection      *conn,
                    Eet_Data_Descriptor *edd,
                    const void          *data_in,
                    const char          *cipher_key)
{
   void *flat_data;
   int data_size;
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, EINA_FALSE);

   flat_data = eet_data_descriptor_encode_cipher(edd,
                                                 data_in,
                                                 cipher_key,
                                                 &data_size);
   if (!flat_data) return EINA_FALSE;
   if (_eet_connection_raw_send(conn, flat_data, data_size)) ret = EINA_TRUE;
   free(flat_data);
   return ret;
}

EAPI Eina_Bool
eet_connection_node_send(Eet_Connection *conn,
                         Eet_Node       *node,
                         const char     *cipher_key)
{
   void *data;
   int data_size;
   Eina_Bool ret = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, EINA_FALSE);

   data = eet_data_node_encode_cipher(node, cipher_key, &data_size);
   if (!data) return EINA_FALSE;
   if (_eet_connection_raw_send(conn, data, data_size))
     ret = EINA_TRUE;
   free(data);
   return ret;
}

EAPI void *
eet_connection_close(Eet_Connection *conn,
                     Eina_Bool      *on_going)
{
   void *user_data;

   if (!conn) return NULL;
   if (on_going) *on_going = conn->received == 0 ? EINA_FALSE : EINA_TRUE;
   user_data = conn->user_data;
   free(conn->buffer);
   free(conn);
   return user_data;
}

