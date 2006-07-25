/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <stdlib.h>
#include <string.h>

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

static void _ecore_dbus_message_length_append(Ecore_DBus_Message *msg, unsigned int size);

static void
_ecore_dbus_message_length_append(Ecore_DBus_Message *msg, unsigned int size)
{
   if (msg->length + size >= msg->size)
     {
	size = (((msg->size + size) / 128) + 1) * 128;

	msg->buffer = realloc(msg->buffer, size);
	memset(msg->buffer + msg->size, 0, size - msg->size);
	msg->size = size;
     }
}

void
_ecore_dbus_message_padding(Ecore_DBus_Message *msg, unsigned int size)
{
   unsigned int        padding;

   padding = msg->length % size;
   if (padding != 0)
     {
	padding = size - padding;
	_ecore_dbus_message_length_append(msg, padding);
	msg->length += padding;
     }
}

void
_ecore_dbus_message_padding_skip(Ecore_DBus_Message *msg, unsigned int size)
{
   unsigned int        padding;

   padding = msg->length % size;
   if (padding != 0)
     {
	padding = size - padding;
	/* We append the padding, so that we are sure that the buffer
	 * is at least length in size
	 */
	_ecore_dbus_message_length_append(msg, padding);
	msg->length += padding;
     }
}

void
_ecore_dbus_message_append_bytes(Ecore_DBus_Message *msg, unsigned char *c, unsigned int size)
{
   _ecore_dbus_message_length_append(msg, size);
   memcpy(msg->buffer + msg->length, c, size);
   msg->length += size;
}

void
_ecore_dbus_message_append_byte(Ecore_DBus_Message *msg, unsigned char c)
{
   _ecore_dbus_message_length_append(msg, 1);
   msg->buffer[msg->length++] = c;
}

void
_ecore_dbus_message_append_uint32(Ecore_DBus_Message *msg, unsigned int i)
{
   unsigned char      *c;

   c = (unsigned char *)&i;
   _ecore_dbus_message_length_append(msg, 4);
   msg->buffer[msg->length++] = c[0];
   msg->buffer[msg->length++] = c[1];
   msg->buffer[msg->length++] = c[2];
   msg->buffer[msg->length++] = c[3];
}

unsigned char
_ecore_dbus_message_read_byte(Ecore_DBus_Message *msg)
{
   unsigned char c;

   _ecore_dbus_message_length_append(msg, 1);
   c = *(unsigned char *)(msg->buffer + msg->length);
   msg->length++;
   return c;
}

unsigned int
_ecore_dbus_message_read_uint32(Ecore_DBus_Message *msg)
{
   unsigned int i;

   _ecore_dbus_message_length_append(msg, 4);
   i = *(unsigned int *)(msg->buffer + msg->length);
   msg->length += 4;
   return i;
}

int
_ecore_dbus_alignment_get(Ecore_DBus_Data_Type type)
{
   switch (type)
     {
        case ECORE_DBUS_DATA_TYPE_BYTE:
        case ECORE_DBUS_DATA_TYPE_SIGNATURE:
        case ECORE_DBUS_DATA_TYPE_VARIANT:
	   return 1;
        case ECORE_DBUS_DATA_TYPE_INT16:
        case ECORE_DBUS_DATA_TYPE_UINT16:
	   return 2;
        case ECORE_DBUS_DATA_TYPE_BOOLEAN:
        case ECORE_DBUS_DATA_TYPE_INT32:
        case ECORE_DBUS_DATA_TYPE_UINT32:
        case ECORE_DBUS_DATA_TYPE_STRING:
        case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
        case ECORE_DBUS_DATA_TYPE_ARRAY:
	   return 4;
        case ECORE_DBUS_DATA_TYPE_INT64:
        case ECORE_DBUS_DATA_TYPE_UINT64:
        case ECORE_DBUS_DATA_TYPE_DOUBLE:
	   return 8;
        case ECORE_DBUS_DATA_TYPE_STRUCT:
        case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
	   return 8;
	default:
	   printf("Ecore_DBus: Alignment requested for invalid data type!\n");
	   return 0;
     }
}
